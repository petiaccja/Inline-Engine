#include "SchedulerGPU.hpp"

#include "ResourceResidencyQueue.hpp"

#include <functional>


namespace inl::gxeng {


void SchedulerGPU::SetPipeline(const Pipeline& pipeline) {
	m_pipeline = &pipeline;
}


void SchedulerGPU::SetJobScheduler(jobs::Scheduler& scheduler) {
	m_scheduler = &scheduler;
}


void SchedulerGPU::BeginFrame(const FrameContext& context) {
	if (m_currentContext) {
		throw InvalidCallException("First finalize previous frame by EndFrame.");
	}
	m_currentContext = std::make_shared<FrameContext>(context);
	m_enqueueCoro = m_scheduler->Enqueue(&SchedulerGPU::EnqueueCoro, this, std::ref(*m_scheduler));
}


jobs::Future<void> SchedulerGPU::Enqueue(std::unique_ptr<BasicCommandList> commandList, std::unique_ptr<VolatileViewHeap> vheap) {
	assert(commandList);
	if (!m_currentContext) {
		throw InvalidCallException("First call BeginFrame.");
	}
	jobs::UniqueLock lk(m_mtx);
	co_await lk.Lock();
	m_queue.push(QueueItem{ std::move(commandList), std::move(vheap), eItemFlag::EXECUTE, m_currentContext });
	m_cvar.NotifyOne();
}


jobs::Future<void> SchedulerGPU::EndFrame(bool successful) {
	if (!m_currentContext) {
		throw InvalidCallException("First call BeginFrame.");
	}
	jobs::UniqueLock lk(m_mtx);
	co_await lk.Lock();
	m_queue.push(QueueItem{ nullptr, nullptr, eItemFlag::END_FRAME, nullptr });
	lk.Unlock();
	m_cvar.NotifyOne();
	co_await m_enqueueCoro;
	m_currentContext = {};
}


jobs::Future<void> SchedulerGPU::EnqueueCoro(SchedulerGPU* self, jobs::Scheduler& scheduler) {
	bool runSession = true;
	ListEnqueuer enqueuer(*self->m_currentContext);
	do {
		jobs::UniqueLock lk(self->m_mtx);
		co_await lk.Lock();
		co_await self->m_cvar.Wait(lk, [self] { return !self->m_queue.empty(); });

		auto first = std::move(self->m_queue.front());
		self->m_queue.pop();

		lk.Unlock();

		switch (first.flag) {
			case eItemFlag::EXECUTE:
				enqueuer(std::move(first.commandList), std::move(first.vheap));
				break;
			case eItemFlag::END_FRAME:
				enqueuer.Present();
				runSession = false;
				break;
		}
	} while (runSession);
}


ListEnqueuer::ListEnqueuer(const FrameContext& context)
	: m_context(context) {}


void ListEnqueuer::operator()(std::unique_ptr<BasicCommandList> commandList, std::unique_ptr<VolatileViewHeap> currentVheap) {
	// Process current list.
	auto currentList = commandList->Decompose();
	auto barriers = GetTransitionBarriers(currentList.usedResources);
	UpdateResourceStates(currentList.usedResources);

	// Submit barriers.
	if (!barriers.empty()) {
		if (!m_prevList.commandList) {
			m_prevList.commandAllocator = m_context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
			m_prevList.commandList = m_context.commandListPool->RequestGraphicsList(m_prevList.commandAllocator.get());
		}
		auto* prevCopyList = dynamic_cast<gxapi::ICopyCommandList*>(m_prevList.commandList.get());
		prevCopyList->ResourceBarrier((unsigned)barriers.size(), barriers.data());
	}

	// Enqueue previous list.
	if (m_prevList.commandList) {
		dynamic_cast<gxapi::ICopyCommandList*>(m_prevList.commandList.get())->Close();
		auto usedResource = GetUsedResources(m_prevList.usedResources, m_prevList.additionalResources);

		SendToQueue(*m_context.commandQueue,
			m_context,
			std::move(m_prevList.commandList),
			std::move(usedResource),
			std::move(m_prevList.scratchSpaces),
			std::move(m_prevList.commandAllocator),
			std::move(m_prevVheap));
	}

	// Save current list for barrier injection of the next list.
	m_prevList = std::move(currentList);
	m_prevVheap = std::move(currentVheap);
}


void ListEnqueuer::Present() {
	// Set backBuffer to PRESENT state.
	Texture2D backBuffer = m_context.backBuffer;
	gxapi::eResourceState backBufferState = backBuffer.ReadState(0);
	if (backBufferState != gxapi::eResourceState::PRESENT) {
		if (!m_prevList.commandList) {
			m_prevList.commandAllocator = m_context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
			m_prevList.commandList = m_context.commandListPool->RequestGraphicsList(m_prevList.commandAllocator.get());
		}

		gxapi::TransitionBarrier barrier(m_context.backBuffer._GetResourcePtr(), backBufferState, gxapi::eResourceState::PRESENT);
		dynamic_cast<gxapi::ICopyCommandList*>(m_prevList.commandList.get())->ResourceBarrier(barrier);
		backBuffer.RecordState(gxapi::eResourceState::PRESENT);
	}

	// Enqueue last command list, if exists.
	if (m_prevList.commandList) {
		dynamic_cast<gxapi::ICopyCommandList*>(m_prevList.commandList.get())->Close();

		std::vector<MemoryObject> usedResources = GetUsedResources(std::move(m_prevList.usedResources), std::move(m_prevList.additionalResources));

		SendToQueue(*m_context.commandQueue,
					m_context,
					std::move(m_prevList.commandList),
					std::move(usedResources),
					std::move(m_prevList.scratchSpaces),
					std::move(m_prevList.commandAllocator),
					std::move(m_prevVheap));
	}
}


std::vector<gxapi::ResourceBarrier> ListEnqueuer::GetTransitionBarriers(const std::vector<ResourceUsage>& usages) {
	std::vector<gxapi::ResourceBarrier> barriers;

	// Collect all necessary barriers.
	for (auto usage : usages) {
		MemoryObject& resource = usage.resource;
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


void ListEnqueuer::UpdateResourceStates(const std::vector<ResourceUsage>& usages) {
	for (auto usage : usages) {
		if (usage.subresource == gxapi::ALL_SUBRESOURCES) {
			for (unsigned s = 0; s < usage.resource.GetNumSubresources(); ++s) {
				usage.resource.RecordState(s, usage.lastState);
			}
		}
		else {
			usage.resource.RecordState(usage.subresource, usage.lastState);
		}
	}
}


std::vector<MemoryObject> ListEnqueuer::GetUsedResources(const std::vector<ResourceUsage>& usages, std::vector<MemoryObject> additional) {
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


template <class... Args>
void ListEnqueuer::SendToQueue(CommandQueue& target,
							   const FrameContext& context,
							   CmdListPtr list,
							   std::vector<MemoryObject> usedResources,
							   Args&&... cleanables) {
	// Enqueue CPU task to make resources resident before the command list runs.
	SyncPoint residentPoint = context.residencyQueue->EnqueueInit(usedResources);

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



} // namespace inl::gxeng