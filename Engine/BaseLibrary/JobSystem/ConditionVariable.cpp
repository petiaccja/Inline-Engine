#include "ConditionVariable.hpp"
#include <mutex>
#include "Future.hpp"


namespace inl::jobs {


ConditionVariable::CvarAwaiter::CvarAwaiter(CvarAwaiter&& rhs) 
	: 
	m_mtx(rhs.m_mtx), 
	m_cvar(rhs.m_cvar),
	m_pred(std::move(rhs.m_pred)),
	m_next(rhs.m_next),
	m_awaitingHandle(std::move(rhs.m_awaitingHandle)),
	m_mutexAwaiter(std::move(rhs.m_mutexAwaiter)),
	m_scheduler(rhs.m_scheduler)
{
	rhs.m_awaitingHandle = {};
	rhs.m_next = nullptr;
	rhs.m_scheduler = nullptr;
}

bool ConditionVariable::CvarAwaiter::await_ready() const noexcept {
	// Note that predicate does not need to be checked in await_suspend because it's protected by the mutex.
	if (m_pred) {
		return m_pred();
	}
	return false;
}


bool ConditionVariable::CvarAwaiter::await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept {
	// Coroutine is suspended.
	m_awaitingHandle = awaitingCoroutine;
	m_scheduler = scheduler;

	// Add this to the waiting list.
	bool success;
	do {
		CvarAwaiter* first = m_cvar.m_firstAwaiter;
		m_next = first;
		success = m_cvar.m_firstAwaiter.compare_exchange_weak(m_next, this);
	} while (!success);
	
	// Accessing *this is safe as coro cannot be continued while the mutex is being held.
	m_mtx.Unlock();

	// *this is unsafe to access from here on, it might have been destroyed on another thread.

	return true;
}



ConditionVariable::ConditionVariable() : m_firstAwaiter(nullptr) {
}


ConditionVariable::CvarAwaiter ConditionVariable::Wait(UniqueLock& lock) {
	return CvarAwaiter(*this, lock);
}


void ConditionVariable::WaitExplicit(UniqueLock& lock) {
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
