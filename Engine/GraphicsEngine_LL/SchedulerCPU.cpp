#include "SchedulerCPU.hpp"

#include "SchedulerGPU.hpp"
#include "GraphicsCommandList.hpp"

namespace inl::gxeng {


class UploadTask : public GraphicsTask {
public:
	UploadTask(const std::vector<UploadManager::UploadDescription>* uploads)
		: m_uploads(uploads) {}
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

private:
	const std::vector<UploadManager::UploadDescription>* m_uploads;
};


void UploadTask::Setup(SetupContext& context) {
	return;
}
void UploadTask::Execute(RenderContext& context) {
	CopyCommandList& commandList = context.AsCopy();

	for (auto& request : *m_uploads) {
		// Init copy parameters
		auto& source = request.source;
		auto& destination = request.destination;

		// Set resource states
		commandList.SetResourceState(destination, gxapi::eResourceState::COPY_DEST);

		auto destType = request.destType;

		if (destType == UploadManager::DestType::BUFFER) {
			auto& dstBuffer = static_cast<const LinearBuffer&>(destination);
			commandList.CopyBuffer(dstBuffer, request.dstOffsetX, source, 0, source.GetSize());
		}
		else if (destType == UploadManager::DestType::TEXTURE_2D) {
			auto& dstTexture = static_cast<const Texture2D&>(destination);
			SubTexture2D dstPlace(request.dstSubresource, Vector<intptr_t, 2>((intptr_t)request.dstOffsetX, (intptr_t)request.dstOffsetY));
			commandList.CopyTexture(dstTexture, source, dstPlace, request.textureBufferDesc);
		}
	}
}

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



void SchedulerCPU::SetPipeline(const Pipeline& pipeline) {
	m_pipeline = &pipeline;
}

void SchedulerCPU::SetJobScheduler(jobs::Scheduler& scheduler) {
	m_scheduler = &scheduler;
}


void SchedulerCPU::RunPipeline(const FrameContext& frameContext, SchedulerGPU& schedulerGpu) {
	assert(m_pipeline);

	FrameContextEx frameContextEx;
	static_cast<FrameContext&>(frameContextEx) = frameContext;
	frameContextEx.schedulerGpu = &schedulerGpu;

	schedulerGpu.BeginFrame(frameContext);
	try {
		auto[uploadList, uploadVheap] = ExecuteUploadTask(frameContext);
		if (uploadList) {
			schedulerGpu.Enqueue(std::move(uploadList), std::move(uploadVheap)).get();
		}
		LaunchTasks(frameContextEx, OnSetupNode);
		LaunchTasks(frameContextEx, OnExecuteNode);
		schedulerGpu.EndFrame(true).get();
	}
	catch (...) {
		schedulerGpu.EndFrame(false).get();
		throw; // The main scheduler will deal with it, ehhh?
	}
}


std::vector<lemon::ListDigraph::Node> SchedulerCPU::GetSourceNodes(const lemon::ListDigraph& graph) {
	std::vector<lemon::ListDigraph::Node> sources;

	for (lemon::ListDigraph::NodeIt nodeIt(graph); nodeIt != lemon::INVALID; ++nodeIt) {
		size_t numDependencies = countInArcs(graph, nodeIt);
		if (numDependencies == 0) {
			sources.push_back(nodeIt);
		}
	}

	return sources;
}


void SchedulerCPU::LaunchTasks(const FrameContextEx& context, std::function<jobs::Future<std::any>(const FrameContextEx&, const Pipeline&, lemon::ListDigraph::Node, std::any)> onNode) {
	struct DependencyCounter {
	public:
		explicit DependencyCounter(size_t total = 0) {
			this->total = total;
		}
		void Reset() {
			*counter = 0;
		}
		bool operator++() {
			return (counter->fetch_add(1) + 1) == total;
		}

	private:
		std::shared_ptr<std::atomic_size_t> counter = std::make_shared<std::atomic_size_t>(0);
		std::size_t total = 0;
	};

	lemon::ListDigraph::NodeMap<DependencyCounter> depCounter(m_pipeline->GetTaskGraph());
	for (lemon::ListDigraph::NodeIt nodeIt(m_pipeline->GetTaskGraph()); nodeIt != lemon::INVALID; ++nodeIt) {
		depCounter[nodeIt] = DependencyCounter{ (size_t)lemon::countInArcs(m_pipeline->GetTaskGraph(), nodeIt) };
	}

	auto stopFlag = std::make_shared<std::atomic_bool>(false);
	auto Traverse = [this, &context, stopFlag, &onNode, &depCounter](lemon::ListDigraph::Node node, std::any passThrough, auto self) -> jobs::Future<void> {
		if (stopFlag->load()) {
			co_return;
		}

		try {
			std::any passToNext = co_await onNode(context, *m_pipeline, node, passThrough);

			std::vector<jobs::Future<void>> childJobs;
			for (lemon::ListDigraph::OutArcIt outArc(m_pipeline->GetTaskGraph(), node); outArc != lemon::INVALID; ++outArc) {
				auto nextNode = m_pipeline->GetTaskGraph().target(outArc);

				if (++depCounter[nextNode]) {
					childJobs.push_back(m_scheduler->Enqueue(self, nextNode, passToNext, self));
				}
			}

			for (auto& childFuture : childJobs) {
				co_await childFuture;
			}
		}
		catch (...) {
			stopFlag->store(true);
			throw;
		}
	};

	auto sourceNodes = GetSourceNodes(m_pipeline->GetTaskGraph());
	std::vector<jobs::Future<void>> childJobs;
	for (auto sourceNode : sourceNodes) {
		childJobs.push_back(m_scheduler->Enqueue(Traverse, sourceNode, std::any{}, Traverse));
	}

	std::vector<std::exception_ptr> exceptions;
	for (auto& job : childJobs) {
		try {
			job.get();
		}
		catch (...) {
			exceptions.push_back(std::current_exception());
		}
	}
	if (!exceptions.empty()) {
		// Rest of the exceptions is ignored.
		// TODO: throw some aggregate exception?
		std::rethrow_exception(exceptions[0]);
	}
}


jobs::Future<std::any> SchedulerCPU::OnSetupNode(const FrameContextEx& context, const Pipeline& pipeline, lemon::ListDigraph::Node node, std::any) {
	GraphicsTask* task = pipeline.GetTaskFunctionMap()[node];
	if (task != nullptr) {
		SetupNode(*task, context);
	}
	co_return std::any{};
}


void SchedulerCPU::SetupNode(GraphicsTask& task, const FrameContextEx& context) {
	// Setup given node.
	SetupContext setupContext(context.memoryManager,
							  context.textureSpace,
							  context.rtvHeap,
							  context.dsvHeap,
							  context.shaderManager,
							  context.gxApi);
	task.Setup(setupContext);
}


jobs::Future<std::any> SchedulerCPU::OnExecuteNode(const FrameContextEx& context, const Pipeline& pipeline, lemon::ListDigraph::Node node, std::any forwarded) {
	GraphicsTask* task = pipeline.GetTaskFunctionMap()[node];
	if (task != nullptr) {
		// See if we inherited anything.
		std::optional<ProducedCommands> heritage;
		if (forwarded.has_value()) {
			ProducedCommands& commands = *std::any_cast<std::shared_ptr<ProducedCommands>&>(forwarded); // Throws if wrong.
			heritage = std::move(commands);
		}

		// Execute task.
		ProducedCommands commands = ExecuteNode(*task, heritage, context);

		// Enqueue heritage.
		if (heritage) {
			assert(heritage->list);
			co_await context.schedulerGpu->Enqueue(std::move(heritage->list), std::move(heritage->vheap));
			heritage.reset();
		}

		// Determine if next node can inherit.
		const auto& taskGraph = pipeline.GetTaskGraph();
		bool canNextInherit = false;
		if (lemon::countOutArcs(taskGraph, node) == 1) {
			lemon::ListDigraph::OutArcIt theOnlyOutArc(taskGraph, node);
			lemon::ListDigraph::Node theOnlyNextNode = taskGraph.target(theOnlyOutArc);
			canNextInherit = lemon::countInArcs(taskGraph, theOnlyNextNode) == 1;
		}

		// Pass on command lists to next node if possible.
		if (canNextInherit) {
			std::any ret(std::make_shared<ProducedCommands>(std::move(commands)));
			co_return ret;
		}
		else {
			// Enqueue commands.
			if (commands.list) {
				co_await context.schedulerGpu->Enqueue(std::move(commands.list), std::move(commands.vheap));
			}
		}
	}

	co_return std::any{};
}


SchedulerCPU::ProducedCommands SchedulerCPU::ExecuteNode(GraphicsTask& task, std::optional<ProducedCommands>& inherited, const FrameContextEx& context) {
	// Inherit command list, if available.
	std::unique_ptr<BasicCommandList> inheritedCommandList;
	std::unique_ptr<VolatileViewHeap> inheritedVheap;
	if (inherited) {
		inheritedCommandList = std::move(inherited->list);
		inheritedVheap = std::move(inherited->vheap);
		inherited.reset();
	}

	// Execute given node.
	auto vheap = std::make_unique<VolatileViewHeap>(context.gxApi);
	RenderContext renderContext(context.memoryManager,
								context.textureSpace,
								context.shaderManager,
								context.gxApi,
								context.commandListPool,
								context.commandAllocatorPool,
								context.scratchSpacePool,
								std::move(inheritedCommandList),
								std::move(inheritedVheap));
	renderContext.SetCommandListName(typeid(task).name());
	task.Execute(renderContext);

	// Get inherited and current list, if any.
	std::unique_ptr<BasicCommandList> currentCommandList;
	std::unique_ptr<VolatileViewHeap> currentVheap;
	renderContext.Decompose(inheritedCommandList, currentCommandList, currentVheap);

	if (inheritedCommandList) {
		inherited = ProducedCommands{ std::move(inheritedCommandList), nullptr };
	}
	else {
		inherited.reset();
	}

	return { std::move(currentCommandList), std::move(currentVheap) };
}


} // namespace inl::gxeng