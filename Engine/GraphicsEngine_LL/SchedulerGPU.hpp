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


class SchedulerGPU {
public:
	void SetPipeline(const Pipeline& pipeline);
	void SetJobScheduler(jobs::Scheduler& scheduler);

	void BeginFrame(const FrameContext& context);
	jobs::Future<void> EnqueueCommandList(std::unique_ptr<BasicCommandList> commandList, std::unique_ptr<VolatileViewHeap> vheap);
	jobs::Future<void> FinalizeFrame(bool successful);

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

private:
	template <class T>
	void EnqueueCommandList(CommandQueue& target,
							const FrameContext& context,
							CmdListPtr list,
							std::vector<MemoryObject> usedResources);
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