#pragma once

#include "Scheduler.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

namespace inl::jobs {


class ThreadpoolScheduler : public Scheduler {
public:
	using handle_t = std::experimental::coroutine_handle<>;

	ThreadpoolScheduler(int threadCount = std::thread::hardware_concurrency());
	~ThreadpoolScheduler();

	void Resume(handle_t coroutine) override;

private:
	void ThreadFunc();
private:
	std::vector<std::thread> m_workers;
	std::queue<handle_t> m_tasks;
	std::mutex m_queueMtx;
	std::condition_variable m_cv;
	std::atomic_bool m_running;
};


} // namespace inl::jobs