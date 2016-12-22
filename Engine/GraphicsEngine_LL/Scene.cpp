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

void Scene::SetSun(DirectionalLight* sun) {
	m_sun = sun;
}

const DirectionalLight& Scene::GetSun() const {
	return *m_sun;
}


} // namespace gxeng
} // namespace inl
