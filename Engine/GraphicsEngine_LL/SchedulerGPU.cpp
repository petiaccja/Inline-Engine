#include "SchedulerGPU.hpp"

#include "GraphicsCommandList.hpp"
#include "ResourceResidencyQueue.hpp"
#include "SchedulerCPU.hpp"

#include <queue>


namespace inl::gxeng {

using VolatileViewPtr = std::unique_ptr<VolatileViewHeap>;

struct DecomposedRenderCommand {
	DecomposedRenderCommand() = default;
	DecomposedRenderCommand(CmdAllocPtr commandAllocator,
							CmdListPtr commandList,
							VolatileViewPtr volatileViewHeap,
							std::vector<ScratchSpacePtr> scratchSpaces,
							std::vector<ResourceUsage> usedResources,
							std::vector<MemoryObject> additionalResources)
		: commandAllocator(std::move(commandAllocator)),
		  commandList(std::move(commandList)),
		  volatileViewHeap(std::move(volatileViewHeap)),
		  scratchSpaces(std::move(scratchSpaces)),
		  usedResources(std::move(usedResources)),
		  additionalResources(std::move(additionalResources)) {}
	DecomposedRenderCommand(DecomposedRenderCommand&&) = default;
	DecomposedRenderCommand& operator=(DecomposedRenderCommand&&) = default;
	~DecomposedRenderCommand() {
		if (commandList) {
			dynamic_cast<gxapi::ICopyCommandList*>(commandList.get())->Close();
		}
	}

	CmdAllocPtr commandAllocator;
	CmdListPtr commandList;
	VolatileViewPtr volatileViewHeap;
	std::vector<ScratchSpacePtr> scratchSpaces;
	std::vector<ResourceUsage> usedResources;
	std::vector<MemoryObject> additionalResources;
};


class LinearQueue {
public:
	LinearQueue(const FrameContext& context);
	void operator<<(RenderCommand command);
	void Present();

private:
	/// <summary> Dissects the command list to raw gxapi command list and resource information. </summary>
	DecomposedRenderCommand DecomposeRenderCommand(RenderCommand command);

	/// <summary> Injects transition barriers into <paramref name="subject"/> to match states for <paramref name="next"/>. </summary>
	void InjectBarriers(DecomposedRenderCommand& subject, const DecomposedRenderCommand& next);

	/// <summary> Injects transition barrier into <paramref name="subject"/> to set back-buffer to PRESENT. </summary>
	void FinalizeBackBuffer(DecomposedRenderCommand& subject);

	/// <summary> Submits some command lists to the command queue. </summary>
	/// <param name="chunk"> How many lists to submit at once. </param>
	/// <param name="keep"> How many lists to keep in m_queue. </param>
	void SubmitSome(size_t chunk = 5, size_t keep = 1);

	/// <summary> Enqueues command list, manages init and clean jobs. </summary>
	template <class... Args>
	static void Submit(CommandQueue& target,
					   const FrameContext& context,
					   CmdListPtr list,
					   std::vector<MemoryObject> usedResources,
					   Args&&... cleanables);

	/// <summary> Barriers from the resource's current state to the first usage state. </summary>
	static std::vector<gxapi::ResourceBarrier> TransitionBarriers(const std::vector<ResourceUsage>& usages);

	/// <summary> Update the state of the resources to the last usage state. </summary>
	static void UpdateResourceStates(std::vector<ResourceUsage>& usages);

	/// <summary> Concatenates the two sets of <see cref="MemoryObject"/>s. </summary>
	std::vector<MemoryObject> UsedResources(const std::vector<ResourceUsage>& usages, std::vector<MemoryObject> additional);

private:
	std::queue<DecomposedRenderCommand> m_queue;
	const FrameContext& m_context;
};


LinearQueue::LinearQueue(const FrameContext& context)
	: m_context(context) {}


void LinearQueue::operator<<(RenderCommand command) {
	DecomposedRenderCommand decomp = DecomposeRenderCommand(std::move(command));

	if (m_queue.empty()) {
		m_queue.push({});
	}
	InjectBarriers(m_queue.back(), decomp);
	UpdateResourceStates(decomp.usedResources);

	m_queue.push(std::move(decomp));
	SubmitSome();
}


void LinearQueue::Present() {
	if (m_queue.empty()) {
		m_queue.push({});
	}
	FinalizeBackBuffer(m_queue.back());

	SubmitSome(0, 0);
}

DecomposedRenderCommand LinearQueue::DecomposeRenderCommand(RenderCommand command) {
	auto d = command.list->Decompose();
	return {
		std::move(d.commandAllocator),
		std::move(d.commandList),
		std::move(command.vheap),
		std::move(d.scratchSpaces),
		std::move(d.usedResources),
		std::move(d.additionalResources)
	};
}


std::vector<gxapi::ResourceBarrier> LinearQueue::TransitionBarriers(const std::vector<ResourceUsage>& usages) {
	std::vector<gxapi::ResourceBarrier> barriers;

	for (auto& usage : usages) {
		const MemoryObject& resource = usage.resource;
		unsigned subresource = usage.subresource;
		gxapi::eResourceState targetState = usage.firstState;

		if (subresource != gxapi::ALL_SUBRESOURCES) {
			gxapi::eResourceState sourceState = resource.ReadState(subresource);
			if (sourceState != targetState) {
				barriers.push_back(gxapi::TransitionBarrier{ resource._GetResourcePtr(), sourceState, targetState, subresource });
			}
		}
		else {
			for (unsigned subresourceIdx = 0; subresourceIdx < resource.GetNumSubresources(); ++subresourceIdx) {
				gxapi::eResourceState sourceState = resource.ReadState(subresourceIdx);
				if (sourceState != targetState) {
					barriers.push_back(gxapi::TransitionBarrier{ resource._GetResourcePtr(), sourceState, targetState, subresourceIdx });
				}
			}
		}
	}

	return barriers;
}


void LinearQueue::UpdateResourceStates(std::vector<ResourceUsage>& usages) {
	for (auto& usage : usages) {
		if (usage.subresource != gxapi::ALL_SUBRESOURCES) {
			usage.resource.RecordState(usage.subresource, usage.lastState);
		}
		else {
			for (unsigned s = 0; s < usage.resource.GetNumSubresources(); ++s) {
				usage.resource.RecordState(s, usage.lastState);
			}
		}
	}
}


std::vector<MemoryObject> LinearQueue::UsedResources(const std::vector<ResourceUsage>& usages, std::vector<MemoryObject> additional) {
	std::vector<MemoryObject> usedResourceList = std::move(additional);

	usedResourceList.reserve(usages.size() + additional.size());
	for (auto& v : usages) {
		usedResourceList.push_back(std::move(v.resource));
	}
	for (auto& v : additional) {
		usedResourceList.push_back(std::move(v));
	}

	return usedResourceList;
}

void LinearQueue::InjectBarriers(DecomposedRenderCommand& subject, const DecomposedRenderCommand& next) {
	auto barriers = TransitionBarriers(next.usedResources);

	if (!barriers.empty()) {
		// Create an ad-hoc command list just for the barriers.
		if (!subject.commandList) {
			subject.commandAllocator = m_context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
			subject.commandList = m_context.commandListPool->RequestGraphicsList(subject.commandAllocator.get());
		}
		auto* underlyingList = dynamic_cast<gxapi::IGraphicsCommandList*>(subject.commandList.get());
		underlyingList->ResourceBarrier((unsigned)barriers.size(), barriers.data());
	}	
}

void LinearQueue::FinalizeBackBuffer(DecomposedRenderCommand& subject) {
	Texture2D backBuffer = m_context.backBuffer;
	gxapi::eResourceState backBufferState = backBuffer.ReadState(0);
	if (backBufferState != gxapi::eResourceState::PRESENT) {
		// Create an ad-hoc list.
		if (!subject.commandList) {
			subject.commandAllocator = m_context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
			subject.commandList = m_context.commandListPool->RequestGraphicsList(subject.commandAllocator.get());
		}

		gxapi::TransitionBarrier barrier(m_context.backBuffer._GetResourcePtr(), backBufferState, gxapi::eResourceState::PRESENT);
		dynamic_cast<gxapi::ICopyCommandList*>(subject.commandList.get())->ResourceBarrier(barrier);
		backBuffer.RecordState(gxapi::eResourceState::PRESENT);
	}
}

void LinearQueue::SubmitSome(size_t chunk, size_t keep) {
	while (m_queue.size() > keep) {
		auto& command = m_queue.front();
		if (command.commandList) {
			auto usedResource = UsedResources(command.usedResources, command.additionalResources);

			Submit(*m_context.commandQueue,
				   m_context,
				   std::move(command.commandList),
				   std::move(usedResource),
				   std::move(command.scratchSpaces),
				   std::move(command.commandAllocator),
				   std::move(command.volatileViewHeap));
		}

		m_queue.pop();
	}
}


template <class... Args>
void LinearQueue::Submit(CommandQueue& target,
						 const FrameContext& context,
						 CmdListPtr list,
						 std::vector<MemoryObject> usedResources,
						 Args&&... cleanables) {
	// Enqueue CPU task to make resources resident before the command list runs.
	SyncPoint residentPoint = context.residencyQueue->EnqueueInit(usedResources);

	// Close the command list.
	dynamic_cast<gxapi::ICopyCommandList*>(list.get())->Close();

	// Enqueue the command list itself on the GPU.
	gxapi::ICommandList* execLists[] = {
		list.get(),
	};
	context.commandQueue->Wait(residentPoint);
	context.commandQueue->ExecuteCommandLists(1, execLists);
	SyncPoint completionPoint = context.commandQueue->Signal();

	// Enqueue CPU task to clean up resources after command list finished.
	context.residencyQueue->EnqueueClean(completionPoint, std::move(usedResources), std::forward<Args>(cleanables)...);
}


SchedulerGPU::SchedulerGPU(Pipeline& pipeline) : m_pipeline(pipeline) {
}

void SchedulerGPU::RunPipeline(const FrameContext& frameContext, jobs::Scheduler& scheduler, const SchedulerCPU& cpuScheduler) {
	jobs::SharedFuture<void> fut = EnqueueCommands(frameContext, cpuScheduler);
	fut.Schedule(scheduler);
	fut.get();
}

std::vector<lemon::ListDigraph::Node> SchedulerGPU::SortNodes(const lemon::ListDigraph& taskGraph) {
	std::vector<lemon::ListDigraph::Node> sortedNodes;

	lemon::ListDigraph::NodeMap<int> topologicalOrder(taskGraph);
	topologicalSort(taskGraph, topologicalOrder);

	std::vector<std::pair<int, lemon::ListDigraph::Node>> sortHelper;
	for (lemon::ListDigraph::NodeIt node(taskGraph); node != lemon::INVALID; ++node) {
		sortHelper.push_back({ topologicalOrder[node], node });
	}
	std::sort(sortHelper.begin(), sortHelper.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});

	for (auto [_ignore, node] : sortHelper) {
		sortedNodes.push_back(node);
	}

	return sortedNodes;
}

jobs::SharedFuture<void> SchedulerGPU::EnqueueCommands(const FrameContext& frameContext, const SchedulerCPU& cpuScheduler) {
	auto sortedNodes = SortNodes(m_pipeline.GetTaskGraph());

	auto& commandJobs = cpuScheduler.GetCommandJobs();

	LinearQueue linearQueue{ frameContext };

	linearQueue << UploadResources(frameContext);
	for (auto& node : sortedNodes) {
		RenderCommand& command = co_await * commandJobs[node];
		if (command.list) {
			linearQueue << std::move(command);
		}
	}

	linearQueue.Present();
}


RenderCommand SchedulerGPU::UploadResources(const FrameContext& frameContext) {
	const std::vector<UploadManager::UploadDescription>& uploads = *frameContext.uploadRequests;
	auto vheap = std::make_unique<VolatileViewHeap>(frameContext.gxApi);
	auto clist = std::make_unique<GraphicsCommandList>(frameContext.gxApi,
													   *frameContext.commandListPool,
													   *frameContext.commandAllocatorPool,
													   *frameContext.scratchSpacePool,
													   *frameContext.memoryManager, *vheap);
	for (auto& request : uploads) {
		// Init copy parameters
		auto& source = request.source;
		auto& destination = request.destination;

		// Set resource states
		clist->SetResourceState(destination, gxapi::eResourceState::COPY_DEST);

		auto destType = request.destType;

		if (destType == UploadManager::DestType::BUFFER) {
			auto& dstBuffer = static_cast<const LinearBuffer&>(destination);
			clist->CopyBuffer(dstBuffer, request.dstOffsetX, source, 0, source.GetSize());
		}
		else if (destType == UploadManager::DestType::TEXTURE_2D) {
			auto& dstTexture = static_cast<const Texture2D&>(destination);
			SubTexture2D dstPlace(request.dstSubresource, Vector<intptr_t, 2>((intptr_t)request.dstOffsetX, (intptr_t)request.dstOffsetY));
			clist->CopyTexture(dstTexture, source, dstPlace, request.textureBufferDesc);
		}
	}

	return { std::move(clist), std::move(vheap) };
}


} // namespace inl::gxeng