#include "ThreadpoolScheduler.hpp"

namespace inl::jobs {


ThreadpoolScheduler::ThreadpoolScheduler(int threadCount) {
	m_running = true;
	m_workers.resize(threadCount);
	for (auto& worker : m_workers) {
		worker = std::thread([this] {
			ThreadFunc();
		});
	}
}

ThreadpoolScheduler::~ThreadpoolScheduler() {
	m_running = false;
	m_cv.notify_all();
	for (auto& worker : m_workers) {
		worker.join();
	}
}


void ThreadpoolScheduler::Resume(handle_t coroutine) {
	{
		std::lock_guard<std::mutex> lkg(m_queueMtx);
		m_tasks.push(coroutine);
	}
	m_cv.notify_all();
}


void ThreadpoolScheduler::ThreadFunc() {
	bool hasMore = false;

	while (m_running || hasMore) {
		hasMore = false;
		std::unique_lock<std::mutex> lk(m_queueMtx);
		m_cv.wait(lk, [this] { return !m_running || !m_tasks.empty(); });

		if (!m_tasks.empty()) {
			handle_t handle = m_tasks.front();
			m_tasks.pop();
			hasMore = !m_tasks.empty();

			lk.unlock();

			handle.resume();
		}
	}
}


} // namespace inl::jobs
