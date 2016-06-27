#include "Test.hpp"

#include <BaseLibrary/ScalarLiterals.hpp>
#include <BaseLibrary/RingBuffer.hpp>

#include <iostream>
#include <chrono>
#include <functional>
#include <array>

using namespace std;
using namespace exc::prefix;
using exc::RingBuffer;
using std::chrono::high_resolution_clock;

template <typename T>
using container_type = std::list<T>;

template <typename T>
using TestRingBuffer = RingBuffer<T, container_type<T>>;

static void TestAssertFunc(bool val, const char* expression) {
	if (!val) {
		throw std::runtime_error("Assertion failed while evaluating the following expression:\n"s + expression);
	}
}

#define TestAssert(x) TestAssertFunc(x, #x)

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
			constexpr int count = int(1_mega);

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
			TestAssert(intBuffer.Back() == count-1);
			TestAssert(intBuffer.Front() == count-2);
			
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
			TestAssert(arrayBuffer.Back()[0] == count-1);

			////////////////////////////////

			cout << "----" << endl;

			std::vector<int> intVector;
			intVector.reserve(intBuffer.Count());

			duration = TimedExecution(
				[&intBuffer, &intVector]() {
					auto targetPos = intBuffer.End();
					auto currPos = intBuffer.Begin();
					do {
						intVector.push_back(intBuffer.Front());
						intBuffer.RotateFront();
					} while (++currPos != targetPos);
				}
			);
			cout << "Rotated and copied " << count << " int in " << duration.count() << " sec" << endl;

			TestAssert(intVector[0] == intBuffer.Front());

			////////////////////////////////

			std::vector<std::array<int, 100>> arrayVector;
			arrayVector.reserve(arrayBuffer.Count());

			duration = TimedExecution(
				[&arrayBuffer, &arrayVector]() {
					auto targetPos = arrayBuffer.End();
					auto currPos = arrayBuffer.Begin();
					do {
						arrayVector.push_back(arrayBuffer.Front());
						arrayBuffer.RotateFront();
					} while (++currPos != targetPos);
				}
			);
			cout << "Rotated and copied " << count << " std::array<int, 100> in " << duration.count() << " sec" << endl;


			// Test iterator
			int countedSize = 0;
			int last;
			for (int curr : intBuffer) {
				last = curr;
				countedSize += 1;
			}

			TestAssert(countedSize == intBuffer.Count());
			TestAssert(last == intBuffer.Back());

			countedSize = 0;
			for (auto curr = intBuffer.Begin(); curr != intBuffer.End().AddRounds(2); ++curr) {
				countedSize += 1;
			}

			TestAssert(countedSize == intBuffer.Count()*3);
		}
		catch(std::exception& e) {
			std::cerr << "ERROR: " << e.what() << std::endl;
			return 1;
		}
		catch (...) {
			return 1;
		}

		return 0;
	}
};

