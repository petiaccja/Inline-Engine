#include <BaseLibrary/JobSystem/Future.hpp>
#include <BaseLibrary/JobSystem/Scheduler.hpp>


#include <Catch2/catch.hpp>
#include "BaseLibrary/JobSystem/ThreadpoolScheduler.hpp"


using namespace inl::jobs;



Future<int> DoJob(int arg = 1) {
	co_return arg;
}


Future<int> AddJob(int a, int b) {
	Future<int> futa = DoJob(a);
	Future<int> futb = DoJob(b);

	int ap = co_await futa;
	int bp = co_await futb;
	co_return ap + bp;
}


void PromiseJob(Promise<int> promise) {
	promise.set_value(42);
}


TEST_CASE("JobSystem - Future explicit", "[JobSystem]") {
	ThreadpoolScheduler scheduler(2);
	Future<int> fut = scheduler.Enqueue(DoJob, 1);

	fut.wait();
	int result = fut.get();
	REQUIRE(result == 1);
}


TEST_CASE("JobSystem - Future nested await", "[JobSystem]") {
	ThreadpoolScheduler scheduler(2);
	Future<int> fut = scheduler.Enqueue(AddJob, 5, 6);

	fut.wait();
	int result = fut.get();
	REQUIRE(result == 11);
}


TEST_CASE("JobSystem - Promise explicit", "[JobSystem") {
	ThreadpoolScheduler scheduler(1);

	Promise<int> promise;
	Future<int> future = promise.get_future();

	scheduler.Enqueue(PromiseJob, std::move(promise));

	int result = future.get();
	REQUIRE(result == 42);
}


TEST_CASE("JobSystem - Exception", "[JobSystem]") {
	ThreadpoolScheduler scheduler(2);
	Future<int> fut = scheduler.Enqueue([]() -> Future<int> {
		throw std::runtime_error("Ooops");
		co_return 42;
	});

	REQUIRE_THROWS(fut.get());
}


TEST_CASE("JobSystem - Exception nested", "[JobSystem]") {
	ImmediateScheduler scheduler;

	auto One = []() -> Future<int> {
		throw std::runtime_error("Ooops");
		co_return 42;
	};
	auto Two = [](Scheduler& scheduler, auto functor) -> Future<int> {
		Future<int> one = scheduler.Enqueue(functor);
		co_await one;
		co_return 0;
	};

	Future<int> fut = scheduler.Enqueue(Two, std::ref(scheduler), One);

	REQUIRE_THROWS(fut.get());
}