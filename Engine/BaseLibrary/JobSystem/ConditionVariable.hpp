#pragma once

#include "Fence.hpp"
#include "Mutex.hpp"

#include <future>
#include <optional>
#include <type_traits>


namespace inl::jobs {



class ConditionVariable {
public:
	class CvarAwaiter {
		friend class ConditionVariable;
		using MutexAwaiter = std::invoke_result_t<decltype(&UniqueLock::Lock), UniqueLock*>;

	public:
		CvarAwaiter(CvarAwaiter&&);
		CvarAwaiter(const CvarAwaiter&) = delete;

		bool await_ready() const noexcept;
		template <class T>
		bool await_suspend(T awaitingCoroutine) noexcept;
		void await_resume() noexcept {}

	protected:
		CvarAwaiter(const ConditionVariable& cvar, UniqueLock& mtx, std::function<bool()> pred = {}) noexcept
			: m_cvar(cvar), m_mtx(mtx), m_pred(pred) {}
		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler = nullptr) noexcept;

	private:
		std::experimental::coroutine_handle<> m_awaitingHandle;
		std::function<bool()> m_pred;
		CvarAwaiter* m_next = nullptr;
		Scheduler* m_scheduler = nullptr;
		const ConditionVariable& m_cvar;
		std::optional<MutexAwaiter> m_mutexAwaiter;
		UniqueLock& m_mtx;
	};

public:
	ConditionVariable();

	CvarAwaiter Wait(UniqueLock& lock);
	template <class Predicate>
	CvarAwaiter Wait(UniqueLock& lock, Predicate pred);
	void WaitExplicit(UniqueLock& lock);
	template <class Predicate>
	void WaitExplicit(UniqueLock& lock, Predicate pred);
	void AwakeAwaiter(CvarAwaiter* last);
	void NotifyOne();
	void NotifyAll();

private:
	mutable std::atomic<CvarAwaiter*> m_firstAwaiter;
};


template <class T>
bool ConditionVariable::CvarAwaiter::await_suspend(T awaitingCoroutine) noexcept {
	Scheduler* scheduler = nullptr;
	if constexpr (std::is_base_of_v<SchedulablePromiseTag, std::decay_t<decltype(awaitingCoroutine.promise())>>) {
		scheduler = static_cast<const SchedulablePromiseTag&>(awaitingCoroutine.promise()).m_scheduler;
	}
	return await_suspend(std::experimental::coroutine_handle<>(awaitingCoroutine), scheduler);
}

template <class Predicate>
ConditionVariable::CvarAwaiter ConditionVariable::Wait(UniqueLock& lock, Predicate pred) {
	return CvarAwaiter{ *this, lock, pred };
}

template <class Predicate>
inline void ConditionVariable::WaitExplicit(UniqueLock& lock, Predicate pred) {
	auto wrapper = [](ConditionVariable* self, UniqueLock& lock, Predicate pred) -> std::future<void> {
		co_await self->Wait(lock, std::move(pred));
	};
	auto fut = wrapper(this, lock, std::move(pred));
	fut.get();
}


} // namespace inl::jobs
