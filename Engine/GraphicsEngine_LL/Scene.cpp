#include "Scene.hpp"

#include <cassert>


namespace inl::gxeng {



Scene::Scene(std::string name)
	: m_name(std::move(name)) {}

Scene::~Scene() {}


void Scene::SetName(std::string name) {
	m_name = std::move(name);
}


const std::string& Scene::GetName() const {
	return m_name;
}


std::optional<std::reference_wrapper<EntityCollectionBase>> Scene::GetCollection(std::type_index type) {
	auto it = m_entityCollections.find(type);
	return it != m_entityCollections.end() ? std::ref(*it->second) : std::optional<std::reference_wrapper<EntityCollectionBase>>{};
}


std::optional<std::reference_wrapper<const EntityCollectionBase>> Scene::GetCollection(std::type_index type) const {
	auto it = m_entityCollections.find(type);
	return it != m_entityCollections.end() ? std::ref(*it->second) : std::optional<std::reference_wrapper<EntityCollectionBase>>{};
}


EntityCollectionBase& Scene::AddCollection(std::unique_ptr<EntityCollectionBase> collection) {
	auto [it, fresh] = m_entityCollections.insert({ collection->GetType(), std::move(collection) });
	assert(fresh);
	return std::ref(*it->second);
}

} // namespace inl::gxeng
