#pragma once


#include "Components.hpp"

#include "GameLogic/Hook.hpp"
#include <GameLogic/System.hpp>


class DoubleFooToBarSystem : public inl::game::System<DoubleFooToBarSystem, const FooComponent, BarComponent> {
public:
	void UpdateEntity(float elapsed, const FooComponent& foo, BarComponent& bar) {
		bar.value = 2.0f * foo.value;
	}
};


class StandaloneSystem : public inl::game::System<StandaloneSystem> {
public:
	void Update(float elapsed) override {
		content += "use renewables;";
	}
	std::string content;
};


class MessageSystem : public inl::game::System<MessageSystem> {
public:
	void MakeMessage(std::string message) {
		for (auto& c : message) {
			c = std::toupper(c);
		}
		this->message = message;
	}
	std::string QueryMessage() const {
		return message;
	}

	void Update(float elapsed) override {}

private:
	std::string message;
};


class MessageHook : public inl::game::Hook<MessageSystem> {
public:
	void PreRun(MessageSystem& system) override {
		system.MakeMessage("make me uppercase");
	}
	void PostRun(MessageSystem& system) override {
		message = system.QueryMessage();
	}
	std::string message;
};
