#include "Scene.hpp"


namespace inl {
namespace gxeng {




Scene::Scene(std::string name)
	: m_name(std::move(name))
{}

Scene::~Scene() {}


void Scene::SetName(std::string name) {
	m_name = std::move(name);
}

const std::string& Scene::GetName() const {
	return m_name;
}

EntityCollection<MeshEntity>& Scene::GetMeshEntities() {
	return m_meshEntities;
}

const EntityCollection<MeshEntity>& Scene::GetMeshEntities() const {
	return m_meshEntities;
}



} // namespace gxeng
} // namespace inl
