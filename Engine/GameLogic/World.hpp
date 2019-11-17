#pragma once

#include "ComponentMatrix.hpp"

#include <unordered_map>


namespace inl::game {


class System;
class Entity;


struct EntitySet {
	EntitySet() = default;
	EntitySet(EntitySet&&) = default;
	~EntitySet();
	ContiguousVector<std::unique_ptr<Entity>> entities;
	ComponentMatrix store;
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

	template <class SystemT>
	SystemT& GetSystem();

	/// <summary> Moves all entities form another World into this. </summary>
	World& operator+=(World&& entities);

private:
	template <class Component>
	static size_t MoveEntityExtend(EntitySet& target, EntitySet& source, size_t sourceIndex, Component&& component);
	static size_t MoveEntityPartial(EntitySet& target, EntitySet& source, size_t sourceIndex, size_t skippedComponent);
	static size_t MoveEntity(EntitySet& target, EntitySet& source, size_t sourceIndex);	
	
	
	ComponentScheme GetScheme(const ComponentMatrix& matrix);
	void MergeScheme(const ComponentScheme& scheme, EntitySet&& entitySet);
	void AppendScheme(EntitySet& target, EntitySet&& source);

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
			(newIt->second->store.types.push_back(_ComponentVector<ComponentTypes>{}),...);
		}
		it = newIt;
	}
	if constexpr (sizeof...(ComponentTypes) > 0) {
		it->second->store.entities.emplace_back(std::forward<ComponentTypes>(args)...);
	}
	it->second->entities.push_back(std::make_unique<Entity>(this, it->second.get(), it->second->entities.size()));

	return it->second->entities.back().get();
}


template <class ComponentT>
void World::AddComponent(Entity& entity, ComponentT&& component) {
	assert(entity.GetWorld() == this);

	// STUCK HERE
	// This query and extend scheme and hashtable shit is very slow and pretty ugly...
	// New ComponentMatrix doesn't make this part any better.
	// How to rewrite?

	// Naive implementation of the extended Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<Entity>>&>(entity.GetStore()->entities);
	auto& currentStore = const_cast<ComponentMatrix&>(entity.GetStore()->store);
	const size_t currentIndex = entity.GetIndex();
	const ComponentScheme& currentScheme = GetScheme(currentStore);
	ComponentScheme extendedScheme = currentScheme;
	extendedScheme.Insert(typeid(ComponentT));

	// Find or create extended store.
	auto it = m_componentStores.find(extendedScheme);
	if (it == m_componentStores.end()) {
		auto [newIt, ignore_] = m_componentStores.insert({ extendedScheme, std::make_unique<EntitySet>() });
		newIt->second->store.types = currentStore.types;
		newIt->second->store.types.push_back(_ComponentVector<ComponentT>{});
		it = newIt;
	}
	auto& newStore = it->second->store;

	// Move components.
	auto source = currentStore.entities[currentIndex];
	newStore.entities.push_back(std::move(source));
	const size_t schemeOrderedIndex = extendedScheme.Index(typeid(ComponentT)).second - 1;
	const size_t componentIndex = newStore.types.type_order()[schemeOrderedIndex].second;
	newStore.entities.back().get<ComponentT>(componentIndex) = std::forward<ComponentT>(component);
	currentStore.entities.erase(currentStore.entities.begin() + currentIndex);

	// Update entities.
	it->second->entities.push_back(std::move(currentEntities[currentIndex]));
	currentEntities.erase(currentEntities.begin() + currentIndex);
	entity = Entity(this, it->second.get(), newStore.entities.size() - 1);
	if (currentEntities.size() > currentIndex) {
		*currentEntities[currentIndex] = Entity(this, (EntitySet*)currentEntities[currentIndex]->GetStore(), currentIndex);
	}
}


template <class ComponentT>
void World::RemoveComponent(Entity& entity) {
	assert(entity.GetWorld() == this);

	// Find the index of the first component of specified type.
	auto& currentStore = const_cast<ComponentMatrix&>(entity.GetStore()->store);
	const ComponentScheme& currentScheme = GetScheme(currentStore);

	const auto [firstOfType, lastOfType] = currentScheme.Range(typeid(ComponentT));
	const size_t indexToRemove = std::distance(currentScheme.begin(), firstOfType);
	if (firstOfType == lastOfType) {
		throw InvalidArgumentException("Entity has no such component.");
	}

	RemoveComponent(entity, indexToRemove);
}


template <class SystemT>
SystemT& World::GetSystem() {
	for (auto sys : m_systems) {
		if (auto* typedSys = dynamic_cast<SystemT*>(sys)) {
			return *typedSys;
		}
	}
	throw OutOfRangeException("System of specific type not found.");
}


} // namespace inl::game