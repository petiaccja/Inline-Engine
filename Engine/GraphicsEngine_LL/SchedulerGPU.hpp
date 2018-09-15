#pragma once

#include "BasicCommandList.hpp"
#include "FrameContext.hpp"
#include "Pipeline.hpp"
#include "VolatileViewHeap.hpp"

#include <BaseLibrary/JobSystem/ConditionVariable.hpp>
#include <BaseLibrary/JobSystem/Future.hpp>
#include <BaseLibrary/JobSystem/Scheduler.hpp>

#include <memory>
#include <queue>


namespace inl::gxeng {


// Handles the enqueuing of command lists, including barrier injection, resource state tracking
// and determining if async mode is possible.
class ListEnqueuer {
public:
	ListEnqueuer(const FrameContext& context);
	void operator()(std::unique_ptr<BasicCommandList> commandList, std::unique_ptr<VolatileViewHeap> currentVheap);
	void Present();

	// Command lists (gxeng, not gxapi) do not issue resource barriers for the first time SetResourceState is called.
	// Instead, these states are recorded, and must be "patched in", that is, issued before said command list
	// by the scheduler. This function gives the list of barriers to issue.
	static std::vector<gxapi::ResourceBarrier> GetTransitionBarriers(const std::vector<ResourceUsage>& usages);

	// Goes over the list of resource usages of a command list and updates CPU-side resource state tracking accordingly.
	static void UpdateResourceStates(const std::vector<ResourceUsage>& usages);

	std::vector<MemoryObject> GetUsedResources(const std::vector<ResourceUsage>& usages, std::vector<MemoryObject> additional);

	// Enqueues a command list in target, manages init and clean jobs for the command list.
	template <class... Args>
	void SendToQueue(CommandQueue& target,
		const FrameContext& context,
		CmdListPtr list,
		std::vector<MemoryObject> usedResources,
		Args&&... cleanables);
private:
	BasicCommandList::Decomposition m_prevList;
	std::unique_ptr<VolatileViewHeap> m_prevVheap;
	const FrameContext& m_context;
};


class SchedulerGPU {
public:
	void SetPipeline(const Pipeline& pipeline);
	void SetJobScheduler(jobs::Scheduler& scheduler);

	void BeginFrame(const FrameContext& context);
	jobs::Future<void> Enqueue(std::unique_ptr<BasicCommandList> commandList, std::unique_ptr<VolatileViewHeap> vheap);
	jobs::Future<void> EndFrame(bool successful);

private:
	enum class eItemFlag {
		EXECUTE, // Just queue the command list.
		END_FRAME, // Last item in the frame, not to be queued, exit coro.
	};
	struct QueueItem {
		std::unique_ptr<BasicCommandList> commandList;
		std::unique_ptr<VolatileViewHeap> vheap;
		eItemFlag flag;
		std::shared_ptr<FrameContext> context;
	};
	struct UsedResource {
		MemoryObject* resource;
		unsigned subresource;
		float firstState;
		bool multipleUse;
	};

private:
	// Coroutine running through one frame and enqueues incoming command lists in command queues.
	static jobs::Future<void> EnqueueCoro(SchedulerGPU* self, jobs::Scheduler& scheduler);

private:
	const Pipeline* m_pipeline = nullptr;
	jobs::Scheduler* m_scheduler = nullptr;

	std::shared_ptr<FrameContext> m_currentContext;

	std::queue<QueueItem> m_queue;
	jobs::ConditionVariable m_cvar;
	jobs::Mutex m_mtx;
	jobs::Future<void> m_enqueueCoro;
};



} // namespace inl::gxeng