#pragma once

#include "BasicLevel.hpp"
#include "Scene.hpp"


namespace inl::game {


class ComponentFactory;
class LevelInputArchive;
class LevelOutputArchive;


class Level : BasicLevel {
public:
	Level(Scene& scene);

	void Load(LevelInputArchive& ar, ComponentFactory& factory) override;
	void Save(LevelOutputArchive& ar, ComponentFactory& factory) const override;

private:
	Scene& m_scene;
};



} // namespace inl::game