#include "SchedulerGPU.hpp"

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
		throw InvalidCallException("First finalize previous frame by FinalizeFrame.");
	}
	m_currentContext = std::make_shared<FrameContext>(context);
	m_enqueueCoro = m_scheduler->Enqueue(&SchedulerGPU::EnqueueCoro, this, std::ref(*m_scheduler));
}


jobs::Future<void> SchedulerGPU::EnqueueCommandList(std::unique_ptr<BasicCommandList> commandList, std::unique_ptr<VolatileViewHeap> vheap) {
	if (!m_currentContext) {
		throw InvalidCallException("First call BeginFrame.");
	}
	jobs::UniqueLock lk(m_mtx);
	co_await lk.Lock();
	m_queue.push(QueueItem{ std::move(commandList), std::move(vheap), eItemFlag::EXECUTE, m_currentContext });
}


jobs::Future<void> SchedulerGPU::FinalizeFrame(bool successful) {
	if (!m_currentContext) {
		throw InvalidCallException("First call BeginFrame.");
	}
	jobs::UniqueLock lk(m_mtx);
	co_await lk.Lock();
	m_queue.push(QueueItem{ nullptr, nullptr, eItemFlag::END_FRAME, nullptr });
	m_currentContext = {};
	co_await m_enqueueCoro;
}


jobs::Future<void> SchedulerGPU::EnqueueCoro(SchedulerGPU* self, jobs::Scheduler& scheduler) {
	bool runSession = true;
	do {
		jobs::UniqueLock lk(self->m_mtx);
		co_await lk.Lock();
		co_await self->m_cvar.Wait(lk, [self] { return !self->m_queue.empty(); });

		auto first = std::move(self->m_queue.front());
		self->m_queue.pop();

		lk.Unlock();

		switch (first.flag) {
			case eItemFlag::EXECUTE:
				break;
			case eItemFlag::END_FRAME:
				runSession = false;
				break;
		}
	} while (runSession);
}


} // namespace inl::gxeng