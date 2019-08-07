#include "Scene.hpp"
#include <cassert>


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

/*
EntityCollectionBase* Scene::GetEntities(const std::type_index& entityType) {
	auto it = m_entityCollections.find(entityType);
	if (it != m_entityCollections.end()) {
		return it->second.get();
	}
	else {
		// We don't know the type, so we can't create a new collection.
		// The wrapper must create it.
		return nullptr;
	}
}

const EntityCollectionBase* Scene::GetEntities(const std::type_index& entityType) const {
	auto it = m_entityCollections.find(entityType);
	if (it != m_entityCollections.end()) {
		return it->second.get();
	}
	else {
		// This function may be called concurrently, we don't want concurrent modifications.
		// The wrapper handles this case.
		return nullptr;
	}
}

void Scene::NewCollection(std::unique_ptr<EntityCollectionBase> collection, const std::type_index& type) {
	// Check if collection does not already exist.
	assert(GetEntities(type) == nullptr);

	m_entityCollections.insert({ type, std::move(collection) });
}
*/


} // namespace gxeng
} // namespace inl
