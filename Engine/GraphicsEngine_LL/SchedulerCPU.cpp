#include "SchedulerCPU.hpp"

#include "GraphicsCommandList.hpp"

namespace inl::gxeng {


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

void SchedulerCPU::RunPipeline(const FrameContext& frameContext, jobs::Scheduler& scheduler) {
	// Create jobs.
	CreateSetupJobs(frameContext, scheduler);
	CreateExecuteJobs(frameContext, scheduler);
	CreateCommandJobs(scheduler);

	// Unsuspend all execute jobs.
	// GPU scheduler would do the same eventually, but it may run them in order.
	// We want to run even while the GPU scheduler is processing only the first few results.
	const auto& taskGraph = m_pipeline.GetTaskGraph();
	for (lemon::ListDigraph::NodeIt node(taskGraph); node != lemon::INVALID; ++node) {
		m_executeJobs[node]->Run();
	}
}

auto SchedulerCPU::GetCommandJobs() const -> const lemon::ListDigraph::NodeMap<std::shared_ptr<jobs::SharedFuture<RenderCommand>>>& {
	return m_commandJobs;
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
	RenderCommandCandidate* inherited = nullptr;
	for (lemon::ListDigraph::InArcIt arc(taskGraph, node); arc != lemon::INVALID; ++arc) {
		lemon::ListDigraph::Node dependencyNode = taskGraph.source(arc);

		if (m_listForwarding[arc]) {
			inherited = &(co_await * m_executeJobs[dependencyNode]);
		}
	}
	co_await* m_setupJobs[node];

	// Run task.
	GraphicsTask& task = *m_pipeline.GetTaskFunctionMap()[node];

	std::unique_ptr<BasicCommandList> inheritanceCandidateList = inherited ? std::move(inherited->list) : nullptr;
	std::unique_ptr<VolatileViewHeap> inheritanceCandidateVheap = inherited ? std::move(inherited->vheap) : nullptr;
	;
	RenderContext context{
		frameContext.memoryManager,
		frameContext.textureSpace,
		frameContext.shaderManager,
		frameContext.gxApi,
		frameContext.commandListPool,
		frameContext.commandAllocatorPool,
		frameContext.scratchSpacePool,
		std::move(inheritanceCandidateList),
		std::move(inheritanceCandidateVheap),
	};
	task.Execute(context);

	std::unique_ptr<BasicCommandList> inheritedList;
	std::unique_ptr<BasicCommandList> currentList;
	std::unique_ptr<VolatileViewHeap> currentVheap;
	context.Decompose(inheritedList, currentList, currentVheap);

	co_return RenderCommandCandidate{ std::move(inheritedList), nullptr, std::move(currentList), std::move(currentVheap) };
}

jobs::SharedFuture<RenderCommand> SchedulerCPU::CommandJob(lemon::ListDigraph::Node node) {
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