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
		Mutex& await_resume() noexcept;
	private:
		MutexAwaiter(Mutex& mtx, Fence::FenceAwaiter fenceAwaiter);
	private:
		Fence::FenceAwaiter m_fenceAwaiter;
		Mutex& m_mtx;
	};

public:
	Mutex();
	
	MutexAwaiter operator co_await();
	void lock();
	bool try_lock();
	void unlock();

private:
	Fence m_fence;
	std::atomic_uint64_t m_waitFor;
	std::atomic_uint64_t m_unlockSignal;
	bool m_ignoreNext = false;
};



template <class T>
bool Mutex::MutexAwaiter::await_suspend(T awaitingCoroutine) noexcept {
	return m_fenceAwaiter.await_suspend(awaitingCoroutine);
}


} // namespace inl::jobs