#include <BaseLibrary/JobSystem/Future.hpp>
#include <BaseLibrary/JobSystem/Scheduler.hpp>
#include <BaseLibrary/JobSystem/Mutex.hpp>
#include <BaseLibrary/JobSystem/ConditionVariable.hpp>
#include <BaseLibrary/JobSystem/ThreadpoolScheduler.hpp>


#include <Catch2/catch.hpp>


#include <iostream>

using namespace inl::jobs;
using std::cout;
using std::endl;



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


TEST_CASE("JobSystem - Promise explicit", "[JobSystem]") {
	return;
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


TEST_CASE("JobSystem - Fence signal", "[JobSystem]") {
	ThreadpoolScheduler scheduler(2);
	ImmediateScheduler schedimm;
	Fence fence{0};

	auto func = [&fence]() -> Future<int> {
		auto* address = &fence;
		//co_await fence.Wait(1);
		co_return 10;
	};

	fence.Signal(1);	
	Future<int> fut = scheduler.Enqueue(func);

	REQUIRE(10 == fut.get());
}


TEST_CASE("JobSystem - Fence signal unordered", "[JobSystem]") {
	ThreadpoolScheduler scheduler(2);
	Fence fence{ 0 };

	auto func = [&fence]() -> Future<int> {
		int result = 0;
		co_await fence.Wait(1);
		++result;
		co_await fence.Wait(2);
		++result;
		co_await fence.Wait(3);
		++result;
		co_return result;
	};

	Future<int> fut = scheduler.Enqueue(func);

	fence.Signal(2);
	fence.Signal(3);
	fence.Signal(1);

	Future<int> fut2 = scheduler.Enqueue(func);

	REQUIRE(3 == fut.get());
	REQUIRE(3 == fut2.get());
}


TEST_CASE("JobSystem - Mutex", "[JobSystem]") {
	ThreadpoolScheduler scheduler(4);
	Mutex mutex;
	Fence sync;
	Fence syncBack;

	auto func1 = [&mutex, &sync, &syncBack]() -> Future<void> {
		co_await sync.Wait(1);
		mutex.Lock();
		syncBack.Signal(1);
		co_await sync.Wait(3);
		mutex.Unlock();
	};

	auto func2 = [&mutex, &sync, &syncBack]() -> Future<void> {
		co_await sync.Wait(2);
		if (mutex.TryLock()) {
			throw std::logic_error("Mutex should be locked.");
		}
		syncBack.Signal(2);
		mutex.Lock();
		mutex.Unlock();
	};

	Future<void> fut1 = scheduler.Enqueue(func1);
	Future<void> fut2 = scheduler.Enqueue(func2);

	sync.Signal(1);
	syncBack.WaitExplicit(1);
	sync.Signal(2);
	syncBack.WaitExplicit(2);
	sync.Signal(3);

	REQUIRE_NOTHROW(fut1.get());
	REQUIRE_NOTHROW(fut2.get());
}


TEST_CASE("JobSystem - Condvar notify one", "[JobSystem]") {
	ThreadpoolScheduler scheduler(3);
	Mutex mutex;
	ConditionVariable cvar;
	Fence sync, syncBack;
	std::vector<int> order;

	auto func1 = [&]() -> Future<void> {
		co_await mutex.Lock();
		syncBack.Signal(1);
		co_await cvar.Wait(mutex);

		// We have been notified, mutex must be in locked state.
		if (mutex.TryLock()) {
			throw std::logic_error("Mutex should be locked.");
		}
		order.push_back(1);

		mutex.Unlock();
	};

	auto func2 = [&]() -> Future<void> {
		co_await sync.Wait(1);

		// Just to wait until cvar released the mutex.
		co_await mutex.Lock();
		mutex.Unlock();

		cvar.NotifyOne();

		// See if thread1 ever releases the mutex and the order is correct.
		co_await mutex.Lock();
		order.push_back(2);
		mutex.Unlock();
	};

	auto fut1 = scheduler.Enqueue(func1);
	auto fut2 = scheduler.Enqueue(func2);

	syncBack.WaitExplicit(1);
	sync.Signal(1);

	REQUIRE_NOTHROW(fut1.get());
	REQUIRE_NOTHROW(fut2.get());
	REQUIRE(order.size() == 2);
	REQUIRE(order[0] == 1);
	REQUIRE(order[1] == 2);
}

TEST_CASE("JobSystem - Condvar notify all", "[JobSystem_]") {
	ThreadpoolScheduler scheduler(3);
	Mutex mutex;
	ConditionVariable cvar;
	Fence sync, syncBack[4];
	std::atomic_int counter = 0;
	std::vector<int> order;

	auto func1 = [&]() -> Future<void> {
		int myCount = counter.fetch_add(1);
		cout << "#" << myCount << ": " << "started" << endl;
		
		co_await mutex.Lock();
		syncBack[myCount].Signal(1);
		cout << "#" << myCount << ": " << "waiting..." << endl;
		co_await cvar.Wait(mutex);

		cout << "#" << myCount << ": " << "wait done" << endl;
		order.push_back(myCount);

		// We have been notified, mutex must be in locked state.
		//if (mutex.TryLock()) {
		//	mutex.Unlock();
		//	throw std::logic_error("Mutex should be locked.");
		//}

		mutex.Unlock();
		cout << "#" << myCount << ": " << "exit" << endl;
	};

	auto func2 = [&]() -> Future<void> {
		co_await sync.Wait(1);
		cout << "#" << "M" << ": " << "started" << endl;

		// Just to wait until cvar released the mutex.
		co_await mutex.Lock();
		mutex.Unlock();

		cout << "#" << "M" << ": " << "notifying..." << endl;
		cvar.NotifyAll();

		// See if thread1 ever releases the mutex and the order is correct.
		cout << "#" << "M" << ": " << "locking mutex..." << endl;
		co_await mutex.Lock();
		cout << "#" << "M" << ": " << "locked" << endl;
		order.push_back(5);
		mutex.Unlock();
		cout << "#" << "M" << ": " << "exit" << endl;
	};

	auto fut1_1 = scheduler.Enqueue(func1);
	auto fut1_2 = scheduler.Enqueue(func1);
	auto fut1_3 = scheduler.Enqueue(func1);
	auto fut1_4 = scheduler.Enqueue(func1);
	auto fut2 = scheduler.Enqueue(func2);

	for (auto& s : syncBack) {
		s.WaitExplicit(1);
	}
	sync.Signal(1);

	REQUIRE_NOTHROW(fut1_1.get());
	REQUIRE_NOTHROW(fut1_2.get());
	REQUIRE_NOTHROW(fut1_3.get());
	REQUIRE_NOTHROW(fut1_4.get());
	REQUIRE_NOTHROW(fut2.get());

	REQUIRE(order.size() == 5);
	REQUIRE(order[4] == 5);
}
