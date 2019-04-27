#pragma once


#include "Components.hpp"

#include <GameLogic/System.hpp>


class DoubleFooToBarSystem : public inl::game::SpecificSystem<DoubleFooToBarSystem, const FooComponent, BarComponent> {
public:
	void UpdateEntity(float elapsed, const FooComponent& foo, BarComponent& bar) {
		bar.value = 2.0f * foo.value;
	}
};


class StandaloneSystem : public inl::game::SpecificSystem<StandaloneSystem> {
public:
	void Update(float elapsed) override {
		content += "use renewables;";
	}
	std::string content;
};