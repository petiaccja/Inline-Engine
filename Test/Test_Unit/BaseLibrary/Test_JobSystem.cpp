#include <BaseLibrary/JobSystem/Future.hpp>
#include <BaseLibrary/JobSystem/Scheduler.hpp>
#include <BaseLibrary/JobSystem/Mutex.hpp>
#include <BaseLibrary/JobSystem/ConditionVariable.hpp>
#include <BaseLibrary/JobSystem/ThreadpoolScheduler.hpp>
#include <BaseLibrary/JobSystem/Wait.hpp>

#include <Catch2/catch.hpp>


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
		co_await mutex.Lock();
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
		co_await mutex.Lock();
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
		UniqueLock lk(mutex);
		co_await lk.Lock();
		syncBack.Signal(1);
		co_await cvar.Wait(lk);

		// We have been notified, mutex must be in locked state.
		if (mutex.TryLock()) {
			throw std::logic_error("Mutex should be locked.");
		}
		order.push_back(1);
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


TEST_CASE("JobSystem - Condvar notify all", "[JobSystem]") {
	ThreadpoolScheduler scheduler(3);
	Mutex mutex;
	ConditionVariable cvar;
	Fence sync, syncBack[4];
	std::atomic_int counter = 0;
	std::vector<int> order;

	auto func1 = [&]() -> Future<void> {
		int myCount = counter.fetch_add(1);

		UniqueLock lk(mutex);
		
		co_await lk.Lock();
		syncBack[myCount].Signal(1);
		co_await cvar.Wait(lk);

		order.push_back(myCount);

		// We have been notified, mutex must be in locked state.
		if (mutex.TryLock()) {
			mutex.Unlock();
			throw std::logic_error("Mutex should be locked.");
		}
	};

	auto func2 = [&]() -> Future<void> {
		co_await sync.Wait(1);

		// Just to wait until cvar released the mutex.
		co_await mutex.Lock();
		mutex.Unlock();

		cvar.NotifyAll();

		// See if thread1 ever releases the mutex and the order is correct.
		co_await mutex.Lock();
		order.push_back(5);
		mutex.Unlock();
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


TEST_CASE("JobSystem - WaitAny", "[JobSystem]") {
	ThreadpoolScheduler scheduler(4);
	Fence fence;
	std::atomic<int> counter = 0;
	ConditionVariable cvar[3];
	Mutex mtx[3];
	UniqueLock lk[3] = {
		UniqueLock(mtx[0]),
		UniqueLock(mtx[1]),
		UniqueLock(mtx[2]),
	};
	bool preds[3] = { false, false, false };

	auto func = [&]() -> Future<void> {
		int myId = counter.fetch_add(1);
		co_await fence.Wait(myId);

		UniqueLock lk(mtx[myId]);
		co_await lk.Lock();
		preds[myId] = true;
		lk.Unlock();

		cvar[myId].NotifyAll();
	};

	auto waitFunc = [&]() -> Future<void> {
		co_await lk[0].Lock();
		co_await lk[1].Lock();
		co_await lk[2].Lock();

		co_await WaitAny(
			cvar[0].Wait(lk[0], [&]{ return preds[0]; }),
			cvar[1].Wait(lk[1], [&]{ return preds[1]; }),
			cvar[2].Wait(lk[2], [&]{ return preds[2]; })
		);
	};

	auto fut1 = scheduler.Enqueue(func);
	auto fut2 = scheduler.Enqueue(func);
	auto fut3 = scheduler.Enqueue(func);

	auto wfut = scheduler.Enqueue(waitFunc);

	fence.Signal(1);
	wfut.get();

	fence.Signal(2);
	fut1.get();
	fut2.get();
	fut3.get();
}