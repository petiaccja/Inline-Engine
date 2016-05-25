#include "Test.hpp"
#include <BaseLibrary/Memory/SameSizeAllocatorEngine.hpp>
#include <BaseLibrary/Memory/SlabAllocatorEngine.hpp>
#include <thread>
#include <iostream>
#include <stack>
#include <random>
#include <algorithm>

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
	// create a pool
	constexpr int PoolSize = 1000'000;

	exc::SlabAllocatorEngine engine(PoolSize);

	int idx = engine.Allocate();
	engine.Deallocate(idx);

	std::mutex coutLock;
	std::thread threads[1];

	// benchmark allocation time
	cout << "Benchmark:" << endl;
	auto startTime = std::chrono::high_resolution_clock::now();

	try {
		while (true) {
			size_t index = engine.Allocate();
		}
	}
	catch (std::bad_alloc& ex) {
		//std::lock_guard<std::mutex> lk(coutLock);
		//cout << ex.what() << endl;
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	cout << "Time = " << std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1e6 << " ms" << endl;
	cout << "Allocs = " << PoolSize << endl << endl;


	// test repeated allocations
	cout << "Consistency:" << endl;
	engine.Reset();
	std::vector<int> counter(PoolSize, 0);
	std::vector<int> allocations;
	std::mt19937 rne;
	constexpr int BatchSize = PoolSize / 3;
	constexpr int NumCycles = 100;

	startTime = std::chrono::high_resolution_clock::now();
	try {
		// initial load
		for (int i = 0; i < BatchSize; ++i) {
			auto index = engine.Allocate();
			++counter[index];
			allocations.push_back(index);
		}

		// load cycles
		for (int j = 0; j < NumCycles; j++) {
			// allocate stuff
			for (int i = 0; i < BatchSize; ++i) {
				auto index = engine.Allocate();
				++counter[index];
				allocations.push_back(index);
			}

			// shuffle allocations
			std::shuffle(allocations.begin(), allocations.end(), rne);

			// deallocate stuff
			for (int i = 0; i < BatchSize; ++i) {
				auto index = allocations[i];
				engine.Deallocate(index);
				--counter[index];
			}
			allocations.erase(allocations.begin(), allocations.begin() + BatchSize);
		}
	}
	catch (std::bad_alloc&) {
		cout << "There's some heavy shit with this allocator..." << endl;
	}
	endTime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
	cout << "Time = " << elapsed.count() / 1e6 << " ms" << endl;
	cout << "Allocs = " << BatchSize + BatchSize * NumCycles << endl;
	cout << "Deallocs = " << BatchSize * NumCycles << endl;
	cout << "Total = " << BatchSize + 2*BatchSize*NumCycles << endl;
	cout << "Avg. " << (double)elapsed.count() / (BatchSize + 2 * BatchSize*NumCycles) << " nanoseconds/allocation." << endl;

	bool isOk = true;
	for (auto v : counter) {
		if (v != 1 && v != 0) {
			isOk = false;
			//cout << v << endl;
		}
	}
	if (!isOk) {
		cout << "Double allocation." << endl;
	}
	else {
		cout << "OK." << endl;
	}


	return 0;
}