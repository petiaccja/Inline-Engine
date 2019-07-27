#pragma once


#include "ComponentStore.hpp"

#include <unordered_map>


namespace inl::game {


class System;
class Entity;


struct EntitySet {
	~EntitySet();
	ContiguousVector<std::unique_ptr<Entity>> entities;
	ComponentStore store;
};


class World {
public:
	void Update(float elapsed);

	template <class... ComponentTypes>
	Entity* CreateEntity(ComponentTypes&&... args);

	void DeleteEntity(Entity& entity);

	template <class ComponentT>
	void AddComponent(Entity& entity, ComponentT&& component);

	template <class ComponentT>
	void RemoveComponent(Entity& entity);

	void RemoveComponent(Entity& entity, size_t index);

	void SetSystems(std::vector<System*> systems);
	const std::vector<System*>& GetSystems() const;

private:
	std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>> m_componentStores;
	std::vector<System*> m_systems;
};


} // namespace inl::game


#include "Entity.hpp"


namespace inl::game {


template <class... ComponentTypes>
Entity* World::CreateEntity(ComponentTypes&&... args) {
	static const ComponentScheme scheme = { typeid(ComponentTypes)... };

	auto it = m_componentStores.find(scheme);
	if (it == m_componentStores.end()) {
		auto [newIt, ignored_] = m_componentStores.insert(decltype(m_componentStores)::value_type{ scheme, std::make_unique<EntitySet>() });
		if constexpr (sizeof...(ComponentTypes) > 0) {
			newIt->second->store.Extend<ComponentTypes...>();
		}
		it = newIt;
	}
	if constexpr (sizeof...(ComponentTypes) > 0) {
		it->second->store.PushBack(std::forward<ComponentTypes>(args)...);
	}
	it->second->entities.push_back(std::make_unique<Entity>(this, it->second.get(), it->second->entities.size()));

	return it->second->entities.back().get();
}


template <class ComponentT>
void World::AddComponent(Entity& entity, ComponentT&& component) {
	assert(entity.GetWorld() == this);

	// Naive implementation of the extended Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<Entity>>&>(entity.GetStore()->entities);
	auto& currentStore = const_cast<ComponentStore&>(entity.GetStore()->store);
	const size_t currentIndex = entity.GetIndex();
	const ComponentScheme& currentScheme = currentStore.Scheme();
	ComponentScheme extendedScheme = currentScheme;
	extendedScheme.Insert(typeid(ComponentT));

	// Find or create extended store.
	auto it = m_componentStores.find(extendedScheme);
	if (it == m_componentStores.end()) {
		auto [newIt, ignore_] = m_componentStores.insert({ extendedScheme, std::make_unique<EntitySet>() });
		newIt->second->store = currentStore.CloneScheme();
		newIt->second->store.Extend<ComponentT>();
		it = newIt;
	}

	// Splice entity.
	it->second->store.SpliceBackExtend(currentStore, currentIndex, std::forward<ComponentT>(component));
	auto handle = std::move(currentEntities[currentIndex]);
	it->second->entities.push_back(std::move(handle));
	currentEntities.erase(currentEntities.begin() + currentIndex);
	if (currentStore.Size() == 0) {
		m_componentStores.erase(currentStore.Scheme());
	}
	entity = Entity(this, it->second.get(), it->second->store.Size() - 1);
}


template <class ComponentT>
void World::RemoveComponent(Entity& entity) {
	assert(entity.GetWorld() == this);

	// Find the index of the first component of specified type.
	auto& currentStore = const_cast<ComponentStore&>(entity.GetStore()->store);
	const ComponentScheme& currentScheme = currentStore.Scheme();

	const auto [firstOfType, lastOfType] = currentScheme.Range(typeid(ComponentT));
	const size_t indexToRemove = std::distance(currentScheme.begin(), firstOfType);
	if (firstOfType == lastOfType) {
		throw InvalidArgumentException("Entity has no such component.");
	}

	RemoveComponent(entity, indexToRemove);
}


} // namespace inl::game