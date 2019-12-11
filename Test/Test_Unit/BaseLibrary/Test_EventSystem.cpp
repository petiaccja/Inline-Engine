#include <BaseLibrary/EventSystem/EventSystem.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


class Listener : public EventListener<Listener, float, int> {
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


TEST_CASE("Search in listener", "BaseLibrary:EventSystem") {
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

class MessengerA : public EventListener<MessengerA, MessageForA> {
public:
	void operator()(MessageForA msg) {
		value = msg.value;
		Send(MessageForB{ value + 1 });
	}
	float value = 0.0f;
};

class MessengerB : public EventListener<MessengerB, MessageForB> {
public:
	void operator()(MessageForB msg) {
		value = msg.value;
		Send(MessageForA{ value + 1 });
	}
	float value = 0.0f;
};


TEST_CASE("Message passing", "BaseLibrary:EventSystem") {
	auto a = std::make_shared<MessengerA>();
	auto b = std::make_shared<MessengerB>();
	EventSystem system;
	system.Add(a);
	system.Add(b);
	
	system.Send(MessageForA{ 0.0f });
	for (int i = 0; i < 10; ++i) {
		system.Update();
	}
	REQUIRE(a->value == 8);
	REQUIRE(b->value == 9);
}
