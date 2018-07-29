#include "Mutex.hpp"
#include <cassert>
#include <iostream>
#include "Scheduler.hpp"
#include <wrl/wrappers/corewrappers.h>


namespace inl::jobs {


//------------------------------------------------------------------------------
// MutexAwaiter
//------------------------------------------------------------------------------

Mutex::MutexAwaiter::MutexAwaiter(MutexAwaiter&& rhs) noexcept 
	: m_awaitingHandle(std::move(rhs.m_awaitingHandle)),
	m_next(rhs.m_next),
	m_scheduler(rhs.m_scheduler),
	m_mtx(rhs.m_mtx),
	m_wasAwaited(rhs.m_wasAwaited)
{
	rhs.m_awaitingHandle = {};
	rhs.m_next = nullptr;
	rhs.m_scheduler = nullptr;
	rhs.m_wasAwaited = false;
}

Mutex::MutexAwaiter::~MutexAwaiter() {
	if (m_awaitingHandle && !m_wasAwaited) {
		std::terminate(); // Mutex::Lock() must be co_awaited.
	}
}

bool Mutex::MutexAwaiter::await_ready() const noexcept {
	if (m_wasAwaited) {
		std::terminate(); // Object can only be awaited once.
	}
	m_wasAwaited = true;

	// Null in awaiter list means the mutex is free, so try to lock it.
	MutexAwaiter* expected = nullptr;
	bool success = m_mtx.m_firstAwaiter.compare_exchange_strong(expected, const_cast<MutexAwaiter*>(this));
	if (success) {
		m_mtx.m_holder = const_cast<MutexAwaiter*>(this);
	}
	return success;
}


Mutex::MutexAwaiter::MutexAwaiter(Mutex& mtx) 
	: m_mtx(mtx)
{}

bool Mutex::MutexAwaiter::await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept {
	// Coroutine is suspended.
	m_awaitingHandle = awaitingCoroutine;
	m_scheduler = scheduler;

	// Add this to the waiting list.
	bool success;
	MutexAwaiter* first;
	do {
		first = m_mtx.m_firstAwaiter;
		m_next = first;
		success = m_mtx.m_firstAwaiter.compare_exchange_weak(first, this);
	} while (!success);

	// If first is null, nobody was owning the mutex previously.
	// In this case, no Unlock should start running this coro, so accessing *this is safe.
	if (first == nullptr) {
		// We locked the mutex and don't want to suspend the coro.
		// Unlock() will remove this from the queue.
		m_mtx.m_holder = const_cast<MutexAwaiter*>(this);
		return false;
	}

	return true;
}



//------------------------------------------------------------------------------
// Mutex
//------------------------------------------------------------------------------

Mutex::Mutex() noexcept {
	m_firstAwaiter = nullptr;
	m_holder = nullptr;
}


Mutex::~Mutex() {
	if (m_firstAwaiter != nullptr) {
		std::terminate(); // Mutex cannot be destroyed before being unlocked.
	}
}

Mutex::MutexAwaiter Mutex::Lock() {
	return MutexAwaiter(*this);
}


void Mutex::LockExplicit() {
	assert(false);
}


bool Mutex::TryLock() {
	// Null in awaiter list means the mutex is free, so try to lock it.
	MutexAwaiter* expected = nullptr;
	bool success = m_firstAwaiter.compare_exchange_strong(expected, tryLockTag);
	if (success) {
		m_holder = tryLockTag;
	}
	return success;
}


void Mutex::Unlock() {
	// Peek into the list.
	MutexAwaiter* list = m_firstAwaiter;
	assert(list != nullptr); // Null means no one is owning the lock, thus no one should call unlock.

	// Iterate to the end of list to find the last and before last elements.
	MutexAwaiter *prev = nullptr, *last = list;
	while (last != m_holder) {
		prev = last;
		last = last->m_next;
	}

	// Remove last from the chain and awake prev.
	if (prev) {
		m_holder = prev;
		prev->m_next = nullptr;
		if (prev->m_scheduler) {
			prev->m_scheduler->Resume(prev->m_awaitingHandle);
		}
		else {
			prev->m_awaitingHandle.resume();
		}
	}
	// If there was no previous, somebody might have added one since we obtained the list.
	else {
		MutexAwaiter* expected = last;
		bool success = m_firstAwaiter.compare_exchange_strong(expected, nullptr);
		if (!success) {
			// Go to the end of expected, and remove last item.
			prev = nullptr;
			last = expected;
			while (last != m_holder) {
				prev = last;
				last = last->m_next;
			}
			assert(prev != nullptr); // There must be at least 2 elements in there.
			prev->m_next = nullptr;

			// Awake that prev.
			m_holder = prev;
			if (prev->m_scheduler) {
				prev->m_scheduler->Resume(prev->m_awaitingHandle);
			}
			else {
				prev->m_awaitingHandle.resume();
			}
		}
	}
}


//------------------------------------------------------------------------------
// Wrappers
//------------------------------------------------------------------------------


// LockGuard

LockGuard::LockGuard(Mutex& mutex) noexcept
	: m_mutex(mutex)
{}

LockGuard::~LockGuard() noexcept {
	if (m_locked) {
		m_mutex.Unlock();
	}
}

Mutex::MutexAwaiter LockGuard::Lock() {
	m_locked = true;
	return m_mutex.Lock();
}


// UniqueLock

UniqueLock::UniqueLock(Mutex& mutex) noexcept 
	: m_mutex(mutex) 
{}

UniqueLock::~UniqueLock() noexcept {
	if (m_locked) {
		Unlock();
	}
}
Mutex::MutexAwaiter UniqueLock::Lock() {
	m_locked = true;
	return m_mutex.Lock();
}

void UniqueLock::LockExplicit() {
	m_mutex.LockExplicit();
	m_locked = true;
}

bool UniqueLock::TryLock() {
	bool locked = m_mutex.TryLock();
	if (locked) {
		m_locked = true;
	}
	return locked;
}

void UniqueLock::Unlock() {
	m_mutex.Unlock();
	m_locked = false;
}


} // namespace inl::jobs