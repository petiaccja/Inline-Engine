#include "World.hpp"


namespace inl::game {

class Level {
public:
	Level(World& world);

	template <class Archive>
	void Load(Archive& ar);

	template <class Archive>
	void Save(Archive& ar) const;

private:
	World& world;	
};


template <class Archive>
void Level::Save(Archive& ar) const {
}



} // namespace inl::game