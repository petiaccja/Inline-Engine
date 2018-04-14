#pragma once

#include <experimental/coroutine>
#include <cassert>

#include "SchedulablePromiseTag.hpp"


namespace inl::jobs {


class Scheduler;


template <class T>
struct Task {
private:
	struct Awaiter;
public:
	using ReturnType = T;

	template <class U>
	struct result_container {
		void return_value(T value) { m_result = value; }
		T m_result;
	};

	template <>
	struct result_container<void> {
		void return_void() {}
	};
	

	struct promise_type : public SchedulablePromiseTag, public result_container<T> {
		auto initial_suspend();
		auto final_suspend();
		void unhandled_exception();
		Task get_return_object();

		Awaiter* volatile m_awaitingMe = nullptr;
		std::exception_ptr m_exception;
	};
private:
	using handle_type = std::experimental::coroutine_handle<promise_type>;

	struct Awaiter : public result_container<T> {
		Awaiter(handle_type handle) : m_awaitedHandle(handle) {}

		bool await_ready() const noexcept;
		template <class HandleT>
		bool await_suspend(HandleT awaitingCoroutine) noexcept;
		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept;
		T await_resume() noexcept;

		handle_type m_awaitedHandle;
		std::experimental::coroutine_handle<> m_awatingHandle;
		Scheduler* m_scheduler;
		std::exception_ptr m_exception;
	};

public:
	~Task() = default;

	void Schedule(Scheduler& scheduler);
	void Run();

	auto operator co_await() {
		if (m_detached) {
			throw std::logic_error("Detached task handle cannot be awaited.");
		}
		if (m_handle.promise().m_scheduler) {
			m_detached = true;
		}
		return Awaiter{ m_handle };
	}
private:
	Task(handle_type handle) : m_handle(handle) {}
	handle_type m_handle;
	bool m_detached = false;
};



//------------------------------------------------------------------------------
// Promise
//------------------------------------------------------------------------------

template <class T>
auto Task<T>::promise_type::initial_suspend() {
	return std::experimental::suspend_always();
}


template <class T>
void Task<T>::promise_type::unhandled_exception() {
	if (m_awaitingMe) {
		std::exception_ptr ex = std::current_exception();
		assert(ex != nullptr);
		m_exception = ex;
	}
	else {
		std::terminate();
	}
}


template <class T>
Task<T> Task<T>::promise_type::get_return_object() {
	return Task(handle_type::from_promise(*this));
}



//------------------------------------------------------------------------------
// Awaiter
//------------------------------------------------------------------------------

template <class T>
bool Task<T>::Awaiter::await_ready() const noexcept {
	return m_awaitedHandle.done();
}


template <class T>
template <class HandleT>
bool Task<T>::Awaiter::await_suspend(HandleT awaitingCoroutine) noexcept {
	Scheduler* scheduler = nullptr;
	if constexpr (std::is_base_of_v<SchedulablePromiseTag, std::decay_t<decltype(awaitingCoroutine.promise())>>) {
		scheduler = static_cast<const SchedulablePromiseTag&>(awaitingCoroutine.promise()).m_scheduler;
	}
	return await_suspend(std::experimental::coroutine_handle<>(awaitingCoroutine), scheduler);
}


template <class T>
T Task<T>::Awaiter::await_resume() noexcept {
	if (m_exception) {
		std::rethrow_exception(m_exception);
	}
	if constexpr (!std::is_void_v<T>) {
		return this->m_result;
	}
}



//------------------------------------------------------------------------------
// Task
//------------------------------------------------------------------------------

template <class T>
void Task<T>::Schedule(Scheduler& scheduler) {
	if (m_detached) {
		throw std::logic_error("Detached task handle cannot be scheduled.");
	}
	m_handle.promise().m_scheduler = &scheduler;
}


template <class T>
void Task<T>::Run() {
	if (m_detached) {
		throw std::logic_error("Detached task handle cannot be run.");
	}
	if (m_handle.promise().m_scheduler) {
		m_detached = true;
		m_handle.promise().m_scheduler->Resume(m_handle);
	}
	else {
		m_handle.resume();
	}
}


} // namespace inl::jobs

//------------------------------------------------------------------------------
// Extra (need Scheduler.hpp)
//------------------------------------------------------------------------------

#include "Scheduler.hpp"


namespace inl::jobs {

// Dirty hack because release config just doesnt work without this.
template <class T>
void __declspec(noinline) Nothing(T t) {
	volatile T t2 = t;
	for (int i=0; i<1; ++i) {
		++t2;
	}
}

template <class T>
auto Task<T>::promise_type::final_suspend() {
	if (m_awaitingMe) {
		m_awaitingMe->m_exception = std::move(m_exception);
		if constexpr (!std::is_void_v<T>) {
			m_awaitingMe->return_value(this->m_result);
		}
		auto awaitingHandle = m_awaitingMe->m_awatingHandle;
		auto scheduler = m_awaitingMe->m_scheduler;
		if (scheduler) {
			scheduler->Resume(awaitingHandle);
		}
		else {
			awaitingHandle.resume();
		}
	}

	Nothing(this);
	return std::experimental::suspend_never();
}


template <class T>
bool Task<T>::Awaiter::await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept {
	this->m_awatingHandle = awaitingCoroutine;
	m_awaitedHandle.promise().m_awaitingMe = this;
	m_scheduler = scheduler;

	if (m_awaitedHandle.promise().m_scheduler) {
		m_awaitedHandle.promise().m_scheduler->Resume(m_awaitedHandle);
	}
	else {
		m_awaitedHandle.resume();
	}

	return true;
}


} // namespace inl::jobs