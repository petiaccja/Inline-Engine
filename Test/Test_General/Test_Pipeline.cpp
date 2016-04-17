#include "Test.hpp"

#include <iostream>

using namespace std;


class TestPipeline : public AutoRegisterTest<TestPipeline> {
public:
	TestPipeline() {}

	virtual std::string Name() const {
		return "Pipeline";
	}
	virtual int Run() {
		cout << "test test lol" << endl;
		return 0;
	}
private:
	static int a;
};