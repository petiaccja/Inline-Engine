#include "Test.hpp"
#include "BaseLibrary/Memory/MultiInstanceTLS.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

#ifdef _MSC_VER
#include <Windows.h>
#endif

using std::cout;
using std::endl;


class TestUser {
public:
	TestUser() : value(0) {}
	exc::mi_tls<long long> value;
};


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestMultiTLS : public AutoRegisterTest<TestMultiTLS> {
public:
	TestMultiTLS() {}

	static std::string Name() {
		return "Multi Instance TLS";
	}
	virtual int Run() override;
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


int TestMultiTLS::Run() {
	constexpr int NumInstances = 4;
	constexpr int NumThreads = 4;
	static_assert(NumThreads % NumInstances == 0, "These two must be divisible");

	TestUser instances[NumInstances];
	std::thread threads[NumThreads];
	std::mutex mtx;

#ifdef _MSC_VER
	HANDLE hProcess = GetCurrentProcess();
	SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
#endif

	int i = 0;
	auto startTime = std::chrono::high_resolution_clock::now();
	for (auto& thread : threads)
	{
		thread = std::thread([&, i]{

#ifdef _MSC_VER
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif

			volatile thread_local long long v = 0;
			TestUser& myInstance = instances[i/(NumThreads / NumInstances)];
			
			volatile long long& myValue = (volatile long long&)myInstance.value;

			for (long long i = 0; i < 10'000'000; ++i) {				
				++myInstance.value;
				++myInstance.value;
				++myInstance.value;
				++myInstance.value;
				++myInstance.value;

				++myInstance.value;
				++myInstance.value;
				++myInstance.value;
				++myInstance.value;
				++myInstance.value;

				//++v;
				//++v;
				//++v;
				//++v;
				//++v;

				//++v;
				//++v;
				//++v;
				//++v;
				//++v;
			}
			//myInstance.value = v;

			mtx.lock();
			cout << myInstance.value << endl;
			mtx.unlock();
		});
		++i;
	}

	for (auto& thread : threads) {
		thread.join();
	}
	auto endTime = std::chrono::high_resolution_clock::now();

	cout << "Time = " << std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1e6 << " ms" << endl;



	return 0;
}