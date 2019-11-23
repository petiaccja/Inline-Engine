#include "Scene.hpp"


namespace inl::game {

class Level {
public:
	Level(Scene& world);

	template <class Archive>
	void Load(Archive& ar);

	template <class Archive>
	void Save(Archive& ar) const;

private:
	Scene& world;	
};


template <class Archive>
void Level::Save(Archive& ar) const {
}



} // namespace inl::game