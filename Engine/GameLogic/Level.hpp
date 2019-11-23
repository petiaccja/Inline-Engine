#include "Scene.hpp"


namespace inl::game {

class Level {
public:
	Level(Scene& scene);

	template <class Archive>
	void Load(Archive& ar);

	template <class Archive>
	void Save(Archive& ar) const;

private:
	Scene& m_scene;	
};


template <class Archive>
void Level::Save(Archive& ar) const {
}



} // namespace inl::game