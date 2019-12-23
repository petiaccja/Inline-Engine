#include <BaseLibrary/ActionSystem/ActionSystem.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


class Listener : public ActionListener<Listener, float, int> {
public:
	void operator()(float f) {
		this->f = f;
	}
	void operator()(int i) {
		this->i = i;
	}
	float f = 0.0f;
	int i = 0;
};


TEST_CASE("Search in listener", "BaseLibrary:ActionSystem") {
	Listener listener;
	std::any floatValue{ float(1.0f) };
	std::any intValue{ int(2) };
	listener.TryEvent(floatValue);
	listener.TryEvent(intValue);

	REQUIRE(listener.f == 1.0f);
	REQUIRE(listener.i == 2);
}


struct MessageForA {
	float value = 0.0f;
};

struct MessageForB {
	float value = 0.0f;
};

class MessengerA : public ActionListener<MessengerA, MessageForA> {
public:
	void operator()(MessageForA msg) {
		value = msg.value;
		Send(MessageForB{ value + 1 });
	}
	float value = 0.0f;
};

class MessengerB : public ActionListener<MessengerB, MessageForB> {
public:
	void operator()(MessageForB msg) {
		value = msg.value;
		Send(MessageForA{ value + 1 });
	}
	float value = 0.0f;
};


TEST_CASE("Message passing", "BaseLibrary:ActionSystem") {
	auto a = std::make_shared<MessengerA>();
	auto b = std::make_shared<MessengerB>();
	ActionSystem system;
	system.Add(a);
	system.Add(b);
	
	system.Send(MessageForA{ 0.0f });
	for (int i = 0; i < 10; ++i) {
		system.Update();
	}
	REQUIRE(a->value == 8);
	REQUIRE(b->value == 9);
}
