#pragma once

#include "EntityCollection.hpp"
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <cassert>


namespace inl::gxeng {


class IScene {
public:
	virtual ~IScene() = default;

	virtual void SetName(std::string name) = 0;
	virtual const std::string& GetName() const = 0;

	template <class EntityType>
	EntityCollection<EntityType>& GetEntities();
	template <class EntityType>
	const EntityCollection<EntityType>& GetEntities() const;
protected:
	virtual EntityCollectionBase* GetEntities(const std::type_index& entityType) = 0;
	virtual const EntityCollectionBase* GetEntities(const std::type_index& entityType) const = 0;
	virtual void NewCollection(std::unique_ptr<EntityCollectionBase> collection, const std::type_index& type) = 0;
};


template <class EntityType>
EntityCollection<EntityType>& IScene::GetEntities() {
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
const EntityCollection<EntityType>& IScene::GetEntities() const {
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




} // namespace inl::gxeng