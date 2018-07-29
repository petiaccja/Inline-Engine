#pragma once

#include <iostream>
#include "Future.hpp"
#include "Fence.hpp"


namespace inl::jobs {


namespace impl {

	inline void WaitAnyHelper(std::shared_ptr<Fence> fence) 
	{}

	template <class Head, class... Awaitable>
	void WaitAnyHelper(std::shared_ptr<Fence> fence, Head head, Awaitable&&... awaitables) {
		auto func = [](std::shared_ptr<Fence> fence, Head head) mutable -> Future<void> {
			co_await head;
			fence->Signal(1);
		};
		auto task = func(fence, std::move(head));
		task.Run();
		WaitAnyHelper(std::move(fence), std::forward<Awaitable>(awaitables)...);
	}
	

	inline auto WaitAllHelper() {
		return std::experimental::suspend_never();
	}


	template <class Head, class... Awaitable>
	Future<void> WaitAllHelper(Head& head, Awaitable&&... awaitables) {
		co_await head;
		co_await WaitAllHelper(std::forward<Awaitable>(awaitables)...);
	}

} // namespace impl



template <class... Awaitable>
Future<void> WaitAny(Awaitable... awaitables) {
	std::shared_ptr<Fence> fence = std::make_shared<Fence>();
	impl::WaitAnyHelper(fence, std::forward<Awaitable>(awaitables)...);
	co_await fence->Wait(1);
}

template <class AwaitableIter>
Future<void> WaitAny(AwaitableIter first, AwaitableIter last) {
	std::shared_ptr<Fence> fence = std::make_shared<Fence>();
	auto it = first;
	while (it != last) {
		auto func = [](std::shared_ptr<Fence> fence, auto awaitable) mutable -> Future<void> {
			co_await awaitable;
			fence->Signal(1);
		};
		auto task = func(fence, *it);
		task.Run();
	}
	co_await fence->Wait(1);
}


template <class... Awaitable>
Future<void> WaitAll(Awaitable&&... awaitables) {
	return impl::WaitAllHelper(std::forward<Awaitable>(awaitables)...);
}




} // namespace inl::jobs