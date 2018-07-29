#pragma once

#include "Future.hpp"
#include <experimental/coroutine>
#include <future>
#include <type_traits>


namespace inl::jobs {


template <class RetType>
struct is_schedulable_task {
	static constexpr bool value = false;
};

template <class T>
struct is_schedulable_task<Future<T>> {
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
	auto Enqueue(Func func, Args... args) {
		static_assert(std::is_invocable<Func, Args...>::value, "Object must be callable with given arguments.");
		auto task = MakeTask(std::move(func), this, std::forward<Args>(args)...);
		task.Run();
		return task;
	}
	
	virtual void Resume(std::experimental::coroutine_handle<> coroutine) = 0;
protected:
	template <class Func, class... Args>
	static auto Wrapper(Func func, Args... args) -> Future<std::invoke_result_t<Func, Args...>> {
		using Ret = std::invoke_result_t<Func, Args...>;
		if constexpr (std::is_void_v<Ret>) {
			func(std::forward<Args>(args)...);
			co_await std::experimental::suspend_never(); // co_return; does not work somehow
			co_return;
		}
		else {
			co_return func(std::forward<Args>(args)...);
		}
	}

	template <class Func, class... Args>
	static auto MakeTask(Func func, Scheduler* scheduler, Args... args) {
		if constexpr (is_schedulable<Func, Args...>::value) {
			auto task = [](Func func, Scheduler* scheduler, Args... args) -> std::invoke_result_t<Func, Args...> {
				auto innerTask = func(std::forward<Args>(args)...);
				innerTask.Schedule(*scheduler);
				co_return co_await innerTask;
			}(std::move(func), scheduler, std::forward<Args>(args)...);
			//auto task = func(std::forward<Args>(args)...);
			return task;
		}
		else {
			auto task = Wrapper(std::move(func), std::forward<Args>(args)...);
			task.Schedule(*scheduler);
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