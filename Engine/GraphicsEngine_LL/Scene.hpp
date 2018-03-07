#pragma once

#include "EntityCollection.hpp"
#include <string>
#include <typeindex>
#include <cassert>

namespace inl {
namespace gxeng {


class GraphicsEngine;

class MeshEntity;
class OverlayEntity;
class TextEntity;

class DirectionalLight;


class Scene {
public:
	Scene() = default;
	Scene(std::string name);
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	virtual ~Scene();

	void SetName(std::string name);
	const std::string& GetName() const;
		
	EntityCollection<MeshEntity>& GetMeshEntities();
	const EntityCollection<MeshEntity>& GetMeshEntities() const;

	EntityCollection<OverlayEntity>& GetOverlayEntities();
	const EntityCollection<OverlayEntity>& GetOverlayEntities() const;

	EntityCollection<TextEntity>& GetTextEntities();
	const EntityCollection<TextEntity>& GetTextEntities() const;

	EntityCollection<DirectionalLight>& GetDirectionalLights();
	const EntityCollection<DirectionalLight>& GetDirectionalLights() const;

	template <class EntityType>
	EntityCollection<EntityType>& GetEntities();
	template <class EntityType>
	const EntityCollection<EntityType>& GetEntities() const;
protected:
	EntityCollectionBase* GetEntities(const std::type_index& entityType);
	const EntityCollectionBase* GetEntities(const std::type_index& entityType) const;
	void NewCollection(std::unique_ptr<EntityCollectionBase> collection, const std::type_index& type);

private:
	EntityCollection<MeshEntity> m_meshEntities;	
	EntityCollection<OverlayEntity> m_overlayEntities;
	EntityCollection<TextEntity> m_textEntities;
	EntityCollection<DirectionalLight> m_directionalLights;

	std::unordered_map<std::type_index, std::unique_ptr<EntityCollectionBase>> m_entityCollections;

	std::string m_name;
};


template <class EntityType>
EntityCollection<EntityType>& Scene::GetEntities() {
	static const std::type_index type = typeid(std::decay_t<EntityType>);

	EntityCollectionBase* entities = GetEntities(type);
	if (!entities) {
		std::unique_ptr<EntityCollectionBase> newCollection(std::make_unique<EntityCollection<EntityType>>());
		NewCollection(std::move(newCollection), type);
		entities = GetEntities(type);
		assert(entities != nullptr);
	}
	return static_cast<EntityCollection<EntityType>&>(*entities);
}

template <class EntityType>
const EntityCollection<EntityType>& Scene::GetEntities() const {
	static const std::type_index type = typeid(std::decay_t<EntityType>);
	static const EntityCollection<EntityType> emptyCollection;

	const EntityCollectionBase* entities = GetEntities(type);
	if (entities) {
		return static_cast<const EntityCollection<EntityType>&>(*entities);
	}
	else {
		return emptyCollection;
	}
}


} // namespace gxeng
} // namespace inl
