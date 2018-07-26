#include "ConditionVariable.hpp"
#include <mutex>
#include "Future.hpp"


namespace inl::jobs {


bool ConditionVariable::CvarAwaiter::await_ready() const noexcept {
	return false;
}


bool ConditionVariable::CvarAwaiter::await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept {
	// Coroutine is suspended.
	m_awaitingHandle = awaitingCoroutine;
	m_scheduler = scheduler;

	// We can drop the mutex now.
	m_mtx.Unlock();

	// Add this to the waiting list.
	bool success;
	do {
		CvarAwaiter* first = m_cvar.m_firstAwaiter;
		m_next = first;
		success = m_cvar.m_firstAwaiter.compare_exchange_weak(m_next, this);
	} while (!success);

	// Don't do anything after chaining the coro into the list
	// as *this might have already been deleted from another thread that continued this coro.

	return true;
}



ConditionVariable::ConditionVariable() {
	m_firstAwaiter = nullptr;
}


ConditionVariable::CvarAwaiter ConditionVariable::Wait(Mutex& lock) {
	return CvarAwaiter(*this, lock);
}


void ConditionVariable::WaitExplicit(Mutex& lock) {
	std::future<void> future = [this, &lock]() -> std::future<void> {
		co_await Wait(lock);
	}();
	future.wait();
}


void ConditionVariable::AwakeAwaiter(CvarAwaiter* last) {
	last->m_mutexAwaiter.emplace(last->m_mtx.Lock());

	bool isReady = last->m_mutexAwaiter->await_ready();
	bool isSuspended = true;
	if (!isReady) {
		// Hack it into the mutex's awake queue if mutex could not be acquired immediately.
		isSuspended = last->m_mutexAwaiter->await_suspend(last->m_awaitingHandle, last->m_scheduler);
	}
	if (isReady || !isSuspended) {
		// Resume coroutine if mutex has been acquired immediately.
		if (last->m_scheduler) {
			last->m_scheduler->Resume(last->m_awaitingHandle);
		}
		else {
			last->m_awaitingHandle.resume();
		}
	}
	return;

	if (!last->m_mutexAwaiter->await_ready()) {
		// Hack it into the mutex's awake queue if mutex could not be acquired immediately.
		bool doSuspend = last->m_mutexAwaiter->await_suspend(last->m_awaitingHandle, last->m_scheduler);
	}
	else {
		// Resume coroutine if mutex has been acquired immediately.
		if (last->m_scheduler) {
			last->m_scheduler->Resume(last->m_awaitingHandle);
		}
		else {
			last->m_awaitingHandle.resume();
		}
	}
}

void ConditionVariable::NotifyOne() {
	// Detach head of waiting list.
	CvarAwaiter* waitingList = m_firstAwaiter.exchange(nullptr);

	if (!waitingList) {
		return;
	}

	// Find last item in the list.
	CvarAwaiter *beforeLast = nullptr, *last = waitingList;
	while (last->m_next) {
		beforeLast = last;
		last = last->m_next;
	}

	// Put back whole list, except last.
	if (beforeLast) {
		bool success;
		do {
			CvarAwaiter* first = m_firstAwaiter;
			beforeLast->m_next = first;
			success = m_firstAwaiter.compare_exchange_weak(first, waitingList);
		} while (!success);
	}

	// Awake the last item:
	AwakeAwaiter(last);
}


void ConditionVariable::NotifyAll() {
	// Detach head of waiting list.
	CvarAwaiter* waitingList = m_firstAwaiter.exchange(nullptr);

	if (!waitingList) {
		return;
	}

	// Iterate over the whole list.
	CvarAwaiter* iter = waitingList;
	while (iter) {
		CvarAwaiter* next = iter->m_next;

		// Awake the item:
		AwakeAwaiter(iter);

		iter = next;
	}	
}


}
