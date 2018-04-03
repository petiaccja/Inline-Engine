#pragma once

#include "Task.hpp"
#include <experimental/coroutine>
#include <future>
#include <type_traits>


namespace inl::jobs {


template <class RetType>
struct is_schedulable_task {
	static constexpr bool value = false;
};

template <class T>
struct is_schedulable_task<Task<T>> {
	static constexpr bool value = true;
};

template <class Func, class... Args>
struct is_schedulable {
	static constexpr bool value = is_schedulable_task<std::invoke_result_t<Func, Args...>>::value;
};


class Scheduler {
public:
	virtual ~Scheduler() = default;


	template <class Func, class... Args>
	void Enqueue(Func func, Args... args) {
		static_assert(std::is_invocable<Func, Args...>::value, "Object must be callable with given arguments.");
		auto task = MakeTask(func, std::forward<Args>(args)...);
		task.Schedule(*this);
		task.Run();
	}

	template <class Func, class... Args>
	auto EnqueueTask(Func func, Args... args) {
		static_assert(std::is_invocable<Func, Args...>::value, "Object must be callable with given arguments.");
		auto task = MakeTask(func, std::forward<Args>(args)...);
		task.Schedule(*this);
		return task;
	}

	template <class Func, class... Args>
	auto EnqueueFuture(Func func, Args... args) {
		static_assert(std::is_invocable<Func, Args...>::value, "Object must be callable with given arguments.");
		auto task = MakeTask(func, std::forward<Args>(args)...);
		using ReturnType = typename decltype(task)::ReturnType;
		if constexpr (!std::is_void_v<ReturnType>) {
			auto Wrapper = [task, this]() mutable -> std::future<ReturnType> {
				task.Schedule(*this);
				ReturnType retval = co_await task;
				co_return retval;
			};
			std::future<ReturnType> fut = Wrapper();
			return fut;
		}
		else {
			auto Wrapper = [task, this]() mutable -> std::future<ReturnType> {
				task.Schedule(*this);
				co_await task;
			};
			std::future<ReturnType> fut = Wrapper();
			return fut;
		}
	}

	virtual void Resume(std::experimental::coroutine_handle<> coroutine) = 0;
protected:
	template <class Func, class... Args>
	auto MakeTask(Func func, Args... args) {
		if constexpr (is_schedulable<Func, Args...>::value) {
			// Simply run task.
			auto task = func(std::forward<Args>(args)...);
			return task;
		}
		else {
			// Wrap task into a schedulable coroutine, then run.
			using Ret = std::invoke_result_t<Func, Args...>;
			auto Wrapper = [func, args = std::forward<Args>(args)..., this]() mutable->Task<Ret> {
				co_return func(std::forward<Args>(args)...);
			};
			auto task = Wrapper();
			return task;
		}
	}

};

class ImmediateScheduler : public Scheduler {
public:
	void Resume(std::experimental::coroutine_handle<> coroutine) override {
		if (!coroutine.done()) {
			coroutine.resume();
		}
	}
};


} // namespace inl::jobs