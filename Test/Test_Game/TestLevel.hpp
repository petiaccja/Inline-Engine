#pragma once
#include "ILevel.hpp"


class TestLevel : public ILevel {
public:
	// Loads entire level at once.
	inl::game::Scene Initialize(inl::game::ComponentFactory& componentFactory, inl::Vec3 centerPosition) override;

	// Does nothing, not a tiled level loader.
	inl::game::Scene Expand(inl::game::ComponentFactory& componentFactory, inl::Vec3 centerPosition) override { return {}; }

	// Does nothing, not a tiled level loader.
	void Sweep(inl::Vec3 centerPosition) override {}

	const std::string& GetName() const override;

private:
};
