#include "Test.hpp"
#include <iostream>
#include "BaseLibrary/Delegate.hpp"
#include "BaseLibrary/Event.hpp"


using namespace std;
using namespace inl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestEvent : public AutoRegisterTest<TestEvent> {
public:
	TestEvent() {}

	static std::string Name() {
		return "Event";
	}
	int Run() override;
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


class Source {
public:
	void DoEvent() {
		Evt(1, 2);
	}
	Event<float, float> Evt;
};


float OnEventGlobal1(float a, float b) {
	cout << "g1" << endl;
	return a + b;
}

void OnEventGlobal2(float a, float b) {
	cout << "g2" << endl;
	auto c = a + b;
	(c);
}

class Subscriber {
public:
	float OnEvent1(float a, float b) {
		cout << "m1" << endl;
		return a + b;
	}
	void OnEvent2(float a, float b) {
		cout << "m2" << endl;
		auto c = a + b;
		(c);
	}
};


int TestEvent::Run() {
	Source src;
	Subscriber sub;

	src.Evt += OnEventGlobal1;
	src.Evt += OnEventGlobal2;
	src.Evt += Delegate<void(float, float)>{&Subscriber::OnEvent1, &sub};
	src.Evt += Delegate<void(float, float)>{&Subscriber::OnEvent2, &sub};

	cout << "--- all events ---" << endl;
	src.DoEvent();

	cout << "--- only 2 events ---" << endl;
	src.Evt -= OnEventGlobal2;
	src.Evt -= Delegate<void(float, float)>{&Subscriber::OnEvent2, &sub};
	src.DoEvent();


	cout << "--- same event multiple times ---" << endl;
	src.Evt += OnEventGlobal2;
	src.Evt += OnEventGlobal2;
	src.Evt += OnEventGlobal2;
	src.DoEvent();

	return 0;
}