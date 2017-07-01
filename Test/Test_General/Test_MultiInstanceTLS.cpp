#include "Test.hpp"
#include "BaseLibrary/Memory/MultiInstanceTLS.hpp"
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

#ifdef _MSC_VER
#include <Windows.h>
#endif

using std::cout;
using std::endl;


struct TestData {
	long long value;
	TestData(long long value = 0) : value(value) {
		cout << "data: ctor " << this << endl;
	}
	TestData& operator++() {
		++value;
		return *this;
	}
	TestData(const TestData& rhs) {
		cout << "data: copy " << this << endl;
		value = rhs.value;
	}
	TestData(TestData&& rhs) {
		cout << "data: move " << this << endl;
		value = std::move(rhs.value);
	}
	TestData& operator=(const TestData& rhs) {
		cout << "data: op=copy " << this << endl;
		value = rhs.value;
		return *this;
	}
	TestData& operator=(TestData&& rhs) {
		cout << "data: op=move " << this << endl;
		value = std::move(rhs.value);
		return *this;
	}
	~TestData() {
		cout << "data: dtor " << this << endl;
	}
};

class TestUser {
public:
	TestUser() : value(0) {
		cout << "user: ctor" << endl; 
	}
	TestUser(const TestUser& rhs) {
		cout << "user: copy" << endl;
		value = rhs.value;
	}
	TestUser(TestUser&& rhs) {
		cout << "user: move" << endl;
		value = std::move(rhs.value);
	}
	TestUser& operator=(const TestUser& rhs) {
		cout << "user: op=copy" << endl;
		value = rhs.value;
		return *this;
	}
	TestUser& operator=(TestUser&& rhs) {
		cout << "user: op=move" << endl;
		value = std::move(rhs.value);
		return *this;
	}
	~TestUser() {
		cout << "user: dtor" << endl;
	}
	inl::mi_tls<TestData> value;
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
	constexpr int NumInstances = 1;
	constexpr int NumThreads = 1;
	constexpr long long iterations = 100'000'000;
	static_assert(NumThreads % NumInstances == 0, "These two must be divisible");

	TestUser* instances[NumInstances];
	for (auto& v : instances) {
		v = new TestUser();
	}
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
			TestUser& myInstance = *instances[i/(NumThreads / NumInstances)];

			long long js = iterations / 10;
			for (long long j = 0; j < js; ++j) {
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;

				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
				++(TestData&)myInstance.value;
			}

			mtx.lock();
			cout << ((TestData&)myInstance.value).value << endl;
			delete &myInstance;
			cout << "thread exiting" << endl;
			mtx.unlock();
		});
		++i;
	}

	for (auto& thread : threads) {
		thread.join();
	}
	auto endTime = std::chrono::high_resolution_clock::now();

	double elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1e9; // seconds
	cout << "Time = " << elapsed * 1000 << " ms" << endl;
	cout << "Performance = " << 3.7e9 * elapsed / (iterations) << " cycles / operation" << endl;






	return 0;
}