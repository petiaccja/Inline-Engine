#include "SchedulerCPU.hpp"

#include "GraphicsCommandList.hpp"
#include "SchedulerGPU.hpp"
#include "UploadTask.hpp"

namespace inl::gxeng {



static std::tuple<std::unique_ptr<BasicCommandList>, std::unique_ptr<VolatileViewHeap>> ExecuteUploadTask(const FrameContext& context) {
	UploadTask uploadTask(context.uploadRequests);
	SetupContext setupContext(context.memoryManager,
							  context.textureSpace,
							  context.rtvHeap,
							  context.dsvHeap,
							  context.shaderManager,
							  context.gxApi);
	RenderContext renderContext(context.memoryManager,
								context.textureSpace,
								context.shaderManager,
								context.gxApi,
								context.commandListPool,
								context.commandAllocatorPool,
								context.scratchSpacePool,
								nullptr);
	uploadTask.Setup(setupContext);
	uploadTask.Execute(renderContext);
	std::unique_ptr<BasicCommandList> uploadInherit, uploadList;
	std::unique_ptr<VolatileViewHeap> uploadVheap;
	renderContext.Decompose(uploadInherit, uploadList, uploadVheap);
	return { std::move(uploadList), std::move(uploadVheap) };
}


SchedulerCPU::SchedulerCPU(const Pipeline& pipeline)
	: m_pipeline(pipeline),
	  m_listForwarding(pipeline.GetTaskGraph()),
	  m_setupJobs(pipeline.GetTaskGraph()),
	  m_executeJobs(pipeline.GetTaskGraph()),
	  m_commandJobs(pipeline.GetTaskGraph()) {
	CalculateListForwarding();
	FindTaskGraphSinks();
}

SchedulerCPU::SchedulerCPU(SchedulerCPU&& rhs)
	: m_pipeline(rhs.m_pipeline),
	  m_listForwarding(rhs.m_pipeline.GetTaskGraph()),
	  m_setupJobs(rhs.m_pipeline.GetTaskGraph()),
	  m_executeJobs(rhs.m_pipeline.GetTaskGraph()),
	  m_commandJobs(rhs.m_pipeline.GetTaskGraph()),
	  m_sinks(std::move(rhs.m_sinks)) {
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::ArcIt arc(taskGraph); arc != lemon::INVALID; ++arc) {
		m_listForwarding[arc] = rhs.m_listForwarding[arc];
	}
	for (lemon::ListDigraph::NodeIt node(taskGraph); node != lemon::INVALID; ++node) {
		m_setupJobs[node] = std::move(rhs.m_setupJobs[node]);
		m_executeJobs[node] = std::move(rhs.m_executeJobs[node]);
	}
}

void SchedulerCPU::RunPipeline(const FrameContext& frameContext, jobs::Scheduler& scheduler, SchedulerGPU& schedulerGpu) {
	// Create jobs.
	CreateSetupJobs(frameContext, scheduler);
	CreateExecuteJobs(frameContext, scheduler);

	// Lazily evaluate jobs.
	// This is not necessary, the GPU scheduler should take this task over.
	for (auto& sink : m_sinks) {
		m_executeJobs[sink]->get();
	}

	// Topologically sort the task graph.
	const auto& taskGraph = m_pipeline.GetTaskGraph();
	lemon::ListDigraph::NodeMap<int> topologicalOrder(taskGraph);
	topologicalSort(taskGraph, topologicalOrder);
	std::vector<std::pair<int, lemon::ListDigraph::Node>> nodes;
	for (lemon::ListDigraph::NodeIt node(taskGraph); node != lemon::INVALID; ++node) {
		nodes.push_back({ topologicalOrder[node], node });
	}
	std::sort(nodes.begin(), nodes.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});

	// Send results to the GPU scheduler in the proper topological order.
	schedulerGpu.BeginFrame(frameContext);
	for (auto& [_ingore, node] : nodes) {
		RenderCommandCandidate& task = m_executeJobs[node]->get();

		if (task.inheritedList) {
			schedulerGpu.Enqueue(std::move(task.inheritedList), nullptr).get();
		}
		if (task.list) {
			schedulerGpu.Enqueue(std::move(task.list), std::move(task.vheap)).get();
		}
	}
	schedulerGpu.EndFrame(true).get();
}

void SchedulerCPU::CalculateListForwarding() {
	const auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::ArcIt arcIt(taskGraph); arcIt != lemon::INVALID; ++arcIt) {
		const int numSourceSiblings = countOutArcs(taskGraph, taskGraph.source(arcIt));
		const int numTargetSiblings = countInArcs(taskGraph, taskGraph.target(arcIt));
		m_listForwarding[arcIt] = numSourceSiblings == 1 && numTargetSiblings == 1;
	}
}


void SchedulerCPU::FindTaskGraphSinks() {
	const auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		const int numOutArcs = countOutArcs(taskGraph, nodeIt);
		const bool isSink = numOutArcs == 0;
		if (isSink) {
			m_sinks.push_back(nodeIt);
		}
	}
}


void SchedulerCPU::CreateSetupJobs(const FrameContext& frameContext, jobs::Scheduler& scheduler) {
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		m_setupJobs[nodeIt] = std::make_shared<jobs::SharedFuture<void>>(SetupJob(nodeIt, frameContext));
		m_setupJobs[nodeIt]->Schedule(scheduler);
	}
}


void SchedulerCPU::CreateExecuteJobs(const FrameContext& frameContext, jobs::Scheduler& scheduler) {
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		m_executeJobs[nodeIt] = std::make_shared<jobs::SharedFuture<RenderCommandCandidate>>(ExecuteJob(nodeIt, frameContext));
		m_executeJobs[nodeIt]->Schedule(scheduler);
	}
}


void SchedulerCPU::CreateCommandJobs(jobs::Scheduler& scheduler) {
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		m_commandJobs[nodeIt] = std::make_shared<jobs::SharedFuture<RenderCommand>>(CommandJob(nodeIt));
		// Note that these are short tasks and may be better using immediate scheduling.
		m_commandJobs[nodeIt]->Schedule(scheduler); 
	}
}


jobs::SharedFuture<void> SchedulerCPU::SetupJob(lemon::ListDigraph::Node node, const FrameContext& frameContext) {
	// Wait for dependencies.
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::InArcIt arc(taskGraph, node); arc != lemon::INVALID; ++arc) {
		lemon::ListDigraph::Node dependencyNode = taskGraph.source(arc);
		co_await* m_setupJobs[dependencyNode];
	}

	// Run task.
	GraphicsTask& task = *m_pipeline.GetTaskFunctionMap()[node];

	SetupContext context{
		frameContext.memoryManager,
		frameContext.textureSpace,
		frameContext.rtvHeap,
		frameContext.dsvHeap,
		frameContext.shaderManager,
		frameContext.gxApi
	};
	task.Setup(context);
	co_return;
}


jobs::SharedFuture<SchedulerCPU::RenderCommandCandidate> SchedulerCPU::ExecuteJob(lemon::ListDigraph::Node node, const FrameContext& frameContext) {
	// TODO: just a though: execute jobs need not be in dependency order, only setup jobs
	//		their inputs/outputs are well-defined once the corresponding setup job finished
	//		they only have to wait if they want to inherit the command list
	//		this allows better parallelization

	// Wait for dependencies.
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::InArcIt arc(taskGraph, node); arc != lemon::INVALID; ++arc) {
		lemon::ListDigraph::Node dependencyNode = taskGraph.source(arc);
		// Setup jobs & execute jobs may execute in parallel (for different nodes), hence the extra wait.
		co_await* m_setupJobs[dependencyNode];
		co_await* m_executeJobs[dependencyNode];
	}
	co_await* m_setupJobs[node];

	// Run task.
	GraphicsTask& task = *m_pipeline.GetTaskFunctionMap()[node];

	RenderContext context{
		frameContext.memoryManager,
		frameContext.textureSpace,
		frameContext.shaderManager,
		frameContext.gxApi,
		frameContext.commandListPool,
		frameContext.commandAllocatorPool,
		frameContext.scratchSpacePool,
		nullptr, // No inheritence for now.
		nullptr, // No inheritence for now.
	};
	task.Execute(context);

	std::unique_ptr<BasicCommandList> inheritedList;
	std::unique_ptr<BasicCommandList> currentList;
	std::unique_ptr<VolatileViewHeap> currentVheap;
	context.Decompose(inheritedList, currentList, currentVheap);

	co_return RenderCommandCandidate{ nullptr, nullptr, std::move(currentList), std::move(currentVheap) };
}

jobs::SharedFuture<SchedulerCPU::RenderCommand> SchedulerCPU::CommandJob(lemon::ListDigraph::Node node) {
	// See if dependant refused to inherit. Return its dropped inherited list.
	auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::OutArcIt arc(taskGraph, node); arc != lemon::INVALID; ++arc) {
		if (m_listForwarding[arc]) {
			lemon::ListDigraph::Node dependentNode = taskGraph.target(arc);
			RenderCommandCandidate& candidate = co_await * m_executeJobs[dependentNode];
			if (candidate.inheritedList && candidate.list) {
				co_return { std::move(candidate.inheritedList), std::move(candidate.inheritedVheap) };
			}
		}
	}

	// Just return normally if there was no dependant.
	RenderCommandCandidate& candidate = co_await * m_executeJobs[node];
	co_return { std::move(candidate.list), std::move(candidate.vheap) };
}


} // namespace inl::gxeng