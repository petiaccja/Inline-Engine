#pragma once


#include "Fence.hpp"
#include "SchedulablePromiseTag.hpp"

#include <cassert>
#include <experimental/coroutine>
#include <future>


namespace inl::jobs {


template <class T>
class SharedFuture;

class Scheduler;


//------------------------------------------------------------------------------
// Shared state
//------------------------------------------------------------------------------

template <class T>
struct SharedState {
	SharedState() {
		coroStarted.clear();
	}
	Fence fence;
	T value;
	std::exception_ptr ex;
	std::atomic_flag coroStarted;
};

template <>
struct SharedState<void> {
	SharedState() {
		coroStarted.clear();
	}
	Fence fence;
	std::exception_ptr ex;
	std::atomic_flag coroStarted;
};



//------------------------------------------------------------------------------
// Promise
//------------------------------------------------------------------------------

template <class T>
class PromiseBase {
public:
	PromiseBase();
	PromiseBase(const PromiseBase&) = delete;
	PromiseBase& operator=(const PromiseBase&) = delete;
	PromiseBase(PromiseBase&&) noexcept = default;
	PromiseBase& operator=(PromiseBase&&) noexcept = default;

	SharedFuture<T> get_future();

	void set_exception(std::exception_ptr ex);

protected:
	std::shared_ptr<SharedState<T>> m_sharedState;
};


template <class T>
class Promise : public PromiseBase<T> {
public:
	using PromiseBase<T>::PromiseBase;
	using PromiseBase<T>::operator=;

	void set_value(T value);
};

template <>
class Promise<void> : public PromiseBase<void> {
public:
	using PromiseBase<void>::PromiseBase;
	using PromiseBase<void>::operator=;

	void set_value();
};



//------------------------------------------------------------------------------
// Future
//------------------------------------------------------------------------------

template <class T>
class CoroPromiseBase : public Promise<T> {
public:
	auto initial_suspend() { return std::experimental::suspend_always(); }
	auto final_suspend() { return std::experimental::suspend_never(); }
	//void unhandled_exception() { this->set_exception(std::current_exception()); }
};

template <class T>
class CoroPromise : public CoroPromiseBase<T>, public SchedulablePromiseTag {
public:
	void return_value(T value) { this->set_value(std::move(value)); }
	SharedFuture<T> get_return_object();
};

template <>
class CoroPromise<void> : public CoroPromiseBase<void>, public SchedulablePromiseTag {
public:
	void return_void() { set_value(); }
	SharedFuture<void> get_return_object();
};

template <class T>
class Awaiter {
	template <class T>
	friend class SharedFuture;

public:
	Awaiter(Fence::FenceAwaiter fenceAwaiter) : m_fenceAwaiter(std::move(fenceAwaiter)) {}

	bool await_ready() const noexcept;
	template <class HandleT>
	bool await_suspend(HandleT awaitingCoroutine) noexcept;
	decltype(auto) await_resume();

private:
	Scheduler* m_awatingScheduler = nullptr;
	std::experimental::coroutine_handle<> m_awaitingHandle;
	Fence::FenceAwaiter m_fenceAwaiter;
	SharedFuture<T>* m_future;
};


template <class T>
class SharedFuture {
	friend class PromiseBase<T>;
	friend class CoroPromise<T>;
	friend class Awaiter<T>;

public:
	SharedFuture() = default;
	SharedFuture(const SharedFuture&) = delete;
	SharedFuture& operator=(const SharedFuture&) = delete;
	SharedFuture(SharedFuture&&) noexcept = default;
	SharedFuture& operator=(SharedFuture&&) noexcept = default;
	~SharedFuture() = default;

	bool valid() const noexcept;
	bool ready() const noexcept;
	void wait() const;
	decltype(auto) get() const;

	using promise_type = CoroPromise<T>;
	auto operator co_await() const;

	void Schedule(Scheduler& scheduler);
	void Run();

protected:
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	SharedFuture(handle_type coroutineHandle, std::shared_ptr<SharedState<T>> sharedState)
		: m_handle(std::move(coroutineHandle)), m_sharedState(std::move(sharedState)) {}
	SharedFuture(std::shared_ptr<SharedState<T>> sharedState)
		: m_sharedState(std::move(sharedState)) {}

protected:
	std::shared_ptr<SharedState<T>> m_sharedState;
	mutable bool m_ready = false;
	mutable bool m_alreadyRun = false;
	handle_type m_handle;

public:
	handle_type& TMP_GetHandle() { return m_handle; }
};



//------------------------------------------------------------------------------
// Promise - defs
//------------------------------------------------------------------------------

template <class T>
PromiseBase<T>::PromiseBase() {
	m_sharedState = std::make_shared<SharedState<T>>();
}


template <class T>
SharedFuture<T> PromiseBase<T>::get_future() {
	return SharedFuture<T>{ m_sharedState };
}


template <class T>
void PromiseBase<T>::set_exception(std::exception_ptr ex) {
	m_sharedState->ex = std::move(ex);
	m_sharedState->fence.Signal(1);
}


template <class T>
void Promise<T>::set_value(T value) {
	PromiseBase<T>::m_sharedState->value = std::move(value);
	PromiseBase<T>::m_sharedState->fence.Signal(1);
}

inline void Promise<void>::set_value() {
	m_sharedState->fence.Signal(1);
}


template <class T>
SharedFuture<T> CoroPromise<T>::get_return_object() {
	return SharedFuture<T>{ std::experimental::coroutine_handle<CoroPromise>::from_promise(*this), this->m_sharedState };
}


inline SharedFuture<void> CoroPromise<void>::get_return_object() {
	return SharedFuture<void>{ std::experimental::coroutine_handle<CoroPromise>::from_promise(*this), this->m_sharedState };
}



//------------------------------------------------------------------------------
// Future - defs
//------------------------------------------------------------------------------

template <class T>
bool SharedFuture<T>::valid() const noexcept {
	return (bool)m_sharedState;
}

template <class T>
bool SharedFuture<T>::ready() const noexcept {
	return m_sharedState->fence.TryWait(1);
}

template <class T>
void SharedFuture<T>::wait() const {
	assert(valid());

	// Run if needed.
	bool hasStarted = m_sharedState->coroStarted.test_and_set();
	if (!hasStarted) {
		m_alreadyRun = true;
		Scheduler* scheduler = m_handle.promise().m_scheduler;
		if (scheduler != nullptr) {
			scheduler->Resume(m_handle);
		}
		else {
			m_handle.resume();
		}
	}


	m_sharedState->fence.WaitExplicit(1);
	m_ready = true;
}

template <class T>
decltype(auto) SharedFuture<T>::get() const {
	if (!valid()) {
		throw std::future_error{ std::future_errc::no_state };
	}
	if (!m_ready) {
		wait();
	}
	if (m_sharedState->ex) {
		std::rethrow_exception(m_sharedState->ex);
	}
	if constexpr (!std::is_void_v<T>) {
		return static_cast<T&>(m_sharedState->value);
	}
}

template <class T>
auto SharedFuture<T>::operator co_await() const {
	Awaiter<T> awaiter{ m_sharedState->fence.Wait(1) };
	awaiter.m_future = const_cast<SharedFuture<T>*>(this);
	return awaiter;
}


template <class T>
void SharedFuture<T>::Schedule(Scheduler& scheduler) {
	m_handle.promise().m_scheduler = &scheduler;
}


template <class T>
bool Awaiter<T>::await_ready() const noexcept {
	return m_fenceAwaiter.await_ready();
}

template <class T>
decltype(auto) Awaiter<T>::await_resume() {
	m_fenceAwaiter.await_resume();

	// Handle exceptions.
	std::exception_ptr currentEx = std::current_exception();
	if (currentEx) {
		std::rethrow_exception(std::current_exception());
	}
	else if (m_future->m_sharedState->ex) {
		std::rethrow_exception(m_future->m_sharedState->ex);
	}

	// Return value.
	if constexpr (!std::is_void_v<T>) {
		return static_cast<T&>(m_future->m_sharedState->value);
	}
}



} // namespace inl::jobs

#include "Scheduler.hpp"

namespace inl::jobs {

template <class T>
template <class HandleT>
bool Awaiter<T>::await_suspend(HandleT awaitingCoroutine) noexcept {
	bool hasStarted = m_future->m_sharedState->coroStarted.test_and_set();
	if (!hasStarted) {
		Scheduler* scheduler = m_future->m_handle.promise().m_scheduler;
		m_future->m_alreadyRun = true;
		if (scheduler != nullptr) {
			scheduler->Resume(m_future->m_handle);
		}
		else {
			m_future->m_handle.resume();
		}
	}
	return m_fenceAwaiter.await_suspend(awaitingCoroutine);
}


template <class T>
void SharedFuture<T>::Run() {
	bool hasStarted = m_sharedState->coroStarted.test_and_set();
	if (!hasStarted) {
		m_alreadyRun = true;
		Scheduler* scheduler = m_handle.promise().m_scheduler;
		if (scheduler != nullptr) {
			scheduler->Resume(m_handle);
		}
		else {
			m_handle.resume();
		}
	}
}



} // namespace inl::jobs