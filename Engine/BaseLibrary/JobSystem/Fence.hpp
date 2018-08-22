#pragma once

#include "../SpinMutex.hpp"
#include "SchedulablePromiseTag.hpp"

#include <experimental/coroutine>
#include <atomic>


namespace inl::jobs {


class Scheduler;


class Fence {
public:
	class FenceAwaiter {
		friend class Fence;
	public:
		bool await_ready() const noexcept;
		template <class T>
		bool await_suspend(T awaitingCoroutine) noexcept;
		void await_resume() noexcept {}
	private:
		FenceAwaiter(const Fence& f, uint64_t expected) noexcept;
		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler = nullptr) noexcept;
	private:
		std::experimental::coroutine_handle<> m_awaitingHandle;
		const Fence& m_fence;
		FenceAwaiter* m_next;
		const uint64_t m_targetValue;
		Scheduler* m_scheduler;
	};
public:
	Fence(uint64_t initial = 0);
	Fence(Fence&&) noexcept = default;
	Fence(const Fence&) = delete;
	Fence& operator=(Fence&&) noexcept = default;
	Fence& operator=(const Fence&) = delete;

	void Signal(uint64_t value);
	FenceAwaiter Wait(uint64_t value) const;
	bool TryWait(uint64_t value) const;
	void WaitExplicit(uint64_t value) const;
private:
	std::atomic<uint64_t> m_currentValue;
	mutable FenceAwaiter* m_firstAwaiter;
	mutable SpinMutex m_mtx;
};



template <class T>
bool Fence::FenceAwaiter::await_suspend(T awaitingCoroutine) noexcept {
	Scheduler* scheduler = nullptr;
	if constexpr (std::is_base_of_v<SchedulablePromiseTag, std::decay_t<decltype(awaitingCoroutine.promise())>>) {
		scheduler = static_cast<const SchedulablePromiseTag&>(awaitingCoroutine.promise()).m_scheduler;
	}
	return await_suspend(std::experimental::coroutine_handle<>(awaitingCoroutine), scheduler);
}


} // namespace inl::jobs