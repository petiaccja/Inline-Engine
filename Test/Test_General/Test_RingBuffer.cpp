#include "Test.hpp"

#include <BaseLibrary/RingBuffer.hpp>

#include <iostream>
#include <chrono>
#include <functional>
#include <array>

using namespace std;
using exc::RingBuffer;
using std::chrono::high_resolution_clock;

template <typename T>
using container_type = std::list<T>;

template <typename T>
using TestRingBuffer = RingBuffer<T, container_type<T>>;

void TestAssert(bool val) {
	if (!val) {
		throw std::runtime_error("ERROR (message text of this exception does not matter)");
	}
}


chrono::duration<float, chrono::seconds::period> TimedExecution(std::function<void()> func) {
	auto start = high_resolution_clock::now();

	func();

	return high_resolution_clock::now() - start;
}


class Test_RingBuffer : public AutoRegisterTest<Test_RingBuffer> {
public:
	static std::string Name() {
		return "RingBuffer";
	}

	virtual int Run() override {

		try {
			const int count = 1e5;

			string containerName{typeid(container_type<void>).name()};
			containerName = containerName.substr(0, containerName.find('<'));
			cout << "Testing RingBuffer with container type: " << containerName << endl << endl;

			////////////////////////////////
			
			TestRingBuffer<int> intBuffer;
			auto duration = TimedExecution(
				[&intBuffer, count]() {
					for (int i = 0; i < count; i++) {
						intBuffer.PushFront(i);
					}
				}
			);
			cout << "Pushed " << count << " integers in " << duration.count() << " sec" << endl;

			intBuffer.RotateFront();
			int value = *(--intBuffer.End());
			TestAssert(value == count-1);
			value = *intBuffer.Begin();
			TestAssert(value == count-2);
			
			////////////////////////////////

			TestRingBuffer<std::array<int, 100>> arrayBuffer;
			std::array<int, 100> testArray;

			//just fill with some data
			int i = 0;
			for (auto& curr : testArray) {
				curr = i*3;
			}

			duration = TimedExecution(
				[&arrayBuffer, &testArray, count]() {
					for (int i = 0; i < count; i++) {
						testArray[0] = i;
						arrayBuffer.PushFront(testArray);
					}
				}
			);
			cout << "Pushed " << count << " std::array<int, 100> in " << duration.count() << " sec" << endl;

			arrayBuffer.RotateFront();
			TestAssert((*(--arrayBuffer.End()))[0] == count-1);

			////////////////////////////////

			cout << "----" << endl;

			TestRingBuffer<int> intBuffer2;
			duration = TimedExecution(
				[&intBuffer, &intBuffer2]() {
					auto targetPos = intBuffer.End();
					auto currPos = intBuffer.Begin();
					do {
						intBuffer2.PushFront(intBuffer.Front());
						intBuffer.RotateFront();
					} while (++currPos != targetPos);
				}
			);
			cout << "Rotated and pushed " << count << " int in " << duration.count() << " sec" << endl;

			////////////////////////////////

			TestRingBuffer<std::array<int, 100>> arrayBuffer2;
			duration = TimedExecution(
				[&arrayBuffer, &arrayBuffer2]() {
					auto targetPos = arrayBuffer.End();
					auto currPos = arrayBuffer.Begin();
					do {
						arrayBuffer2.PushFront(arrayBuffer.Front());
						arrayBuffer.RotateFront();
					} while (++currPos != targetPos);
				}
			);
			cout << "Rotated and pushed " << count << " std::array<int, 100> in " << duration.count() << " sec" << endl;
			
		}
		catch (...) {
			return 1;
		}

		return 0;
	}
};

