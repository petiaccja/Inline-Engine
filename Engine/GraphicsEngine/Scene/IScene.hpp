#pragma once

#include "EntityCollection.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <typeindex>


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
	virtual std::optional<std::reference_wrapper<EntityCollectionBase>> GetCollection(std::type_index type) = 0;
	virtual std::optional<std::reference_wrapper<const EntityCollectionBase>> GetCollection(std::type_index type) const = 0;
	virtual EntityCollectionBase& AddCollection(std::unique_ptr<EntityCollectionBase> collection) = 0;
};


template <class EntityType>
EntityCollection<EntityType>& IScene::GetEntities() {
	auto ref = GetCollection(typeid(EntityType));
	if (ref) {
		return static_cast<EntityCollection<EntityType>&>(ref.value().get());
	}
	return static_cast<EntityCollection<EntityType>&>(AddCollection(std::make_unique<EntityCollection<EntityType>>()));
}


template <class EntityType>
const EntityCollection<EntityType>& IScene::GetEntities() const {
	thread_local const EntityCollection<EntityType> empty;
	auto ref = GetCollection(typeid(EntityType));
	return ref ? static_cast<const EntityCollection<EntityType>&>(ref.value().get()) : empty;
}


} // namespace inl::gxeng