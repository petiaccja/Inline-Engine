#pragma once

#include "SchedulablePromiseTag.hpp"
#include <atomic>
#include <experimental/coroutine>


namespace inl::jobs {


class Mutex {
public:
	class MutexAwaiter {
		friend class ConditionVariable;
		friend class Mutex;
	public:
		MutexAwaiter(MutexAwaiter&&) noexcept;
		MutexAwaiter& operator=(MutexAwaiter&&) = delete;
		MutexAwaiter(const MutexAwaiter&) = delete;
		MutexAwaiter& operator=(const MutexAwaiter&) = delete;
		~MutexAwaiter();

		bool await_ready() const noexcept;
		template <class T>
		bool await_suspend(T awaitingCoroutine) noexcept;
		void await_resume() noexcept {}
	private:
		MutexAwaiter(Mutex& mtx);
		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler = nullptr) noexcept;
	private:
		std::experimental::coroutine_handle<> m_awaitingHandle;
		MutexAwaiter* m_next = nullptr;
		Scheduler* m_scheduler = nullptr;
		Mutex& m_mtx;
		mutable bool m_wasAwaited = false;
	};
public:
	Mutex() noexcept;
	Mutex(const Mutex&) = delete;
	Mutex(Mutex&&) noexcept = default;
	Mutex& operator=(const Mutex&) = delete;
	Mutex& operator=(Mutex&&) noexcept = default;
	~Mutex();
	
	MutexAwaiter Lock();
	void LockExplicit();
	bool TryLock();
	void Unlock();
private:
	std::atomic<MutexAwaiter*> m_firstAwaiter; // Nullptr if free, otherwise last in the list owns mutex. Lst is a dangling pointer.
	volatile MutexAwaiter* m_holder; // If same as the last in the list above. Used to figure out where the list ends, because last pointer in list in always dangling.
	inline static MutexAwaiter* tryLockTag = reinterpret_cast<MutexAwaiter*>(~size_t(0));
};


class LockGuard {
public:
	LockGuard(Mutex& mutex) noexcept;
	~LockGuard() noexcept;

	Mutex::MutexAwaiter Lock();
private:
	Mutex& m_mutex;
	bool m_locked = false;
};


class UniqueLock {
public:
	UniqueLock(Mutex& mutex) noexcept;
	~UniqueLock() noexcept;

	Mutex::MutexAwaiter Lock();
	void LockExplicit();
	bool TryLock();
	void Unlock();
private:
	Mutex& m_mutex;
	bool m_locked = false;
};


template <class T>
bool Mutex::MutexAwaiter::await_suspend(T awaitingCoroutine) noexcept {
	Scheduler* scheduler = nullptr;
	if constexpr (std::is_base_of_v<SchedulablePromiseTag, std::decay_t<decltype(awaitingCoroutine.promise())>>) {
		scheduler = static_cast<const SchedulablePromiseTag&>(awaitingCoroutine.promise()).m_scheduler;
	}
	return await_suspend(std::experimental::coroutine_handle<>(awaitingCoroutine), scheduler);
}





} // namespace inl::jobs