#pragma once

#include "Fence.hpp"


namespace inl::jobs {


class Mutex {
public:
	class MutexAwaiter {
		friend class Mutex;
	public:
		bool await_ready() const noexcept;
		template <class T>
		bool await_suspend(T awaitingCoroutine) noexcept;
		void await_resume() noexcept {}
	private:
		MutexAwaiter(Fence::FenceAwaiter fenceAwaiter);
		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler = nullptr) noexcept;
	private:
		Fence::FenceAwaiter m_fenceAwaiter;
	};
public:
	Mutex() noexcept;
	Mutex(const Mutex&) = delete;
	Mutex(Mutex&&) noexcept = default;
	Mutex& operator=(const Mutex&) = delete;
	Mutex& operator=(Mutex&&) noexcept = default;
	
	Fence::FenceAwaiter Lock();
	void LockExplicit();
	bool TryLock();
	void Unlock();
private:
	Fence m_fence;
	std::atomic_uint64_t m_waitFor;
	std::atomic_uint64_t m_unlockSignal;
	bool m_dbg_Locked = false;
};


class LockGuard {
public:
	LockGuard(Mutex& mutex) noexcept;
	~LockGuard() noexcept;

	Fence::FenceAwaiter Lock();
};


class UniqueLock {
public:
	UniqueLock(Mutex& mutex) noexcept;
	~UniqueLock() noexcept;

	Fence::FenceAwaiter Lock();
	void LockExplicit();
	bool TryLock();
	void Unlock();
private:
	Mutex& m_mutex;
};



} // namespace inl::jobs