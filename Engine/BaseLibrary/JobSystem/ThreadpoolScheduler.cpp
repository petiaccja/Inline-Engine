#include "ThreadpoolScheduler.hpp"
#include <BaseLibrary/ThreadName.hpp>
#include <sstream>

namespace inl::jobs {


ThreadpoolScheduler::ThreadpoolScheduler(int threadCount) {
	m_running = true;
	m_workers.resize(threadCount);

	int threadIndex = 0;
	for (auto& worker : m_workers) {
		worker = std::thread([this](int threadIndex) {
			std::stringstream ss;
			ss << "Jobsys Pool #" << threadIndex;
			SetCurrentThreadName(ss.str().c_str());
			ThreadFunc();
		},
		threadIndex);
		++threadIndex;
	}
}

ThreadpoolScheduler::~ThreadpoolScheduler() {
	ShutdownThreads();
}


void ThreadpoolScheduler::Resume(handle_t coroutine) {
	m_tasks2.enqueue(std::move(coroutine));
}


void ThreadpoolScheduler::ShutdownThreads() {
	for (auto& w : m_workers) {
		m_tasks2.enqueue({});
	}
	for (auto& w : m_workers) {
		w.join();
	}
}


void ThreadpoolScheduler::ThreadFunc() {
	bool finishTokenSeen = false;
	do {
		handle_t handle;
		m_tasks2.wait_dequeue(handle);
		if (handle) {
			handle.resume();
		}
		else {
			finishTokenSeen = true;
		}
	} while (!finishTokenSeen);
}


} // namespace inl::jobs
