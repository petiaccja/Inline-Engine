#include "LogPipe.hpp"
#include "LogNode.hpp"

#include <thread>

namespace exc {


LogPipe::LogPipe(LogNode* node) {
	this->node = node;
}

void LogPipe::PutEvent(const Event& evt) {
	if (!node) {
		return;
	}

	// Spin until we're allowed to even try to lock.
	// This is to avoid starvation of LogNode.
	while (node->prohibitPipes) {
		std::this_thread::yield();
	}

	// Deny action while Node does its stuff.
	node->mtx.lock_shared();
	// Deny action while other thread uses this pipe.
	pipeLock.lock();

	buffer.push_back({ std::chrono::high_resolution_clock::now(), evt });

	pipeLock.unlock();
	node->mtx.unlock_shared();

	node->NotifyNewEvent();
}

void LogPipe::PutEvent(Event&& evt) {
	if (!node) {
		return;
	}

	// Spin until we're allowed to even try to lock.
	// This is to avoid starvation of LogNode.
	while (node->prohibitPipes) {
		std::this_thread::yield();
	}

	// Deny action while Node does its stuff.
	node->mtx.lock_shared();
	// Deny action while other thread uses this pipe.
	pipeLock.lock();

	buffer.push_back({ std::chrono::high_resolution_clock::now(), std::move(evt) });

	pipeLock.unlock();
	node->mtx.unlock_shared();

	node->NotifyNewEvent();
}

void LogPipe::Close() {
	node->Flush();
	node->NotifyClose(this);
	node = nullptr;
}


} // namespace exc