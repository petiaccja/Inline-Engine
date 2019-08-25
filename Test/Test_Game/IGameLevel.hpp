#pragma once

#include <string_view>


namespace inl ::game {
class World;
}


class IGameLevel {
public:
	virtual void SetWorld(inl::game::World& world) = 0;
	virtual void Load(std::string_view levelName) = 0;
	virtual void Save(std::ostream& file) = 0;
	virtual void Reset() = 0;
};
