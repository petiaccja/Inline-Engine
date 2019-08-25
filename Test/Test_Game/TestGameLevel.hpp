#pragma once

#include "IGameLevel.hpp"


class TestGameLevel : public IGameLevel {
public:
	void SetWorld(inl::game::World& world) override;
	void Load(std::string_view levelName) override;
	void Save(std::ostream& file) override;
	void Reset() override;

private:
	inl::game::World* m_world = nullptr;
};