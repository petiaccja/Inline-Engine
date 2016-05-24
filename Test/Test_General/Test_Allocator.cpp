#include "Test.hpp"
#include <BaseLibrary/Memory/SameSizeAllocatorEngine.hpp>
#include <BaseLibrary/Memory/SlabAllocatorEngine.hpp>
#include <thread>
#include <iostream>

using std::cout;
using std::endl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestAllocator : public AutoRegisterTest<TestAllocator> {
public:
	TestAllocator() {}

	static std::string Name() {
		return "Allocator";
	}
	virtual int Run() override;
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------

//#define USE_COUNTER_CHECK


int TestAllocator::Run() {
	constexpr int PoolSize = 1'000'000'000;

	exc::SlabAllocatorEngine engine(PoolSize);
#ifdef USE_COUNTER_CHECK
	std::vector<int> counter(PoolSize, 0);
#endif

	int idx = engine.Allocate();
	engine.Deallocate(idx);

	std::mutex coutLock;
	std::thread threads[1];

	auto startTime = std::chrono::high_resolution_clock::now();
	for (auto& thread: threads) {
		thread = std::thread([&] {
			try {
				while (true) {
					size_t index = engine.Allocate();
#ifdef USE_COUNTER_CHECK
					counter[index]++;
#endif
				}
			}
			catch (std::bad_alloc& ex) {
				std::lock_guard<std::mutex> lk(coutLock);
				cout << ex.what() << endl;
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	cout << "Time = " << std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1e6 << " ms" << endl;
	
	bool isOk = true;
#ifdef USE_COUNTER_CHECK
	for (auto v : counter) {
		if (v != 1) {
			isOk = false;
			cout << v << endl;
		}
	}
#endif
	if (!isOk) {
		cout << "Double allocation." << endl;
	}

	return 0;
}