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


} // namespace gxeng
} // namespace inl
