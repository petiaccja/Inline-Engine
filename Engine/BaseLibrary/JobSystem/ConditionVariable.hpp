#pragma once


#include "Fence.hpp"
#include "Mutex.hpp"
#include <optional>
#include <type_traits>


namespace inl::jobs {



class ConditionVariable {
public:
	class CvarAwaiter {
		friend class ConditionVariable;
		using MutexAwaiter = std::invoke_result_t<decltype(&Mutex::Lock), Mutex*>;
	public:
		bool await_ready() const noexcept;
		template <class T>
		bool await_suspend(T awaitingCoroutine) noexcept;
		void await_resume() noexcept {}
	protected:
		CvarAwaiter(const ConditionVariable& cvar, Mutex& mtx) noexcept : m_cvar(cvar), m_mtx(mtx) {}
		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler = nullptr) noexcept;
	private:
		std::experimental::coroutine_handle<> m_awaitingHandle;
		CvarAwaiter* m_next = nullptr;
		Scheduler* m_scheduler = nullptr;
		const ConditionVariable& m_cvar;
		std::optional<MutexAwaiter> m_mutexAwaiter;
		Mutex& m_mtx;
	};
public:
	ConditionVariable();

	CvarAwaiter Wait(Mutex& lock);
	void WaitExplicit(Mutex& lock);
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



}
