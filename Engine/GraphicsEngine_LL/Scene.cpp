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

EntityCollection<OverlayEntity>& Scene::GetOverlayEntities() {
	return m_overlayEntities;
}

const EntityCollection<OverlayEntity>& Scene::GetOverlayEntities() const {
	return m_overlayEntities;
}

EntityCollection<DirectionalLight>& Scene::GetDirectionalLights() {
	return m_directionalLights;
}
const EntityCollection<DirectionalLight>& Scene::GetDirectionalLights() const {
	return m_directionalLights;
}


} // namespace gxeng
} // namespace inl
