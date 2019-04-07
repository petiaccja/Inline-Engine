#pragma once


#include "EntityStore.hpp"

#include <unordered_map>


namespace inl::game {


class World;


struct EntitySet;

// Rename this to normal "Entity"
class SlimEntity {
public:
	SlimEntity() = default;
	SlimEntity(World* world, EntitySet* store, size_t index) : m_world(world), m_store(store), m_index(index) {}




	const World* GetWorld() const { return m_world; }
	const EntitySet* GetStore() const { return m_store; }
	size_t GetIndex() const { return m_index; }

private:
	World* m_world;
	EntitySet* m_store;
	size_t m_index;
};


struct EntitySet {
	ContiguousVector<std::unique_ptr<SlimEntity>> entities;
	EntityStore store;
};


class World {
public:
	void Update(float elapsed);

	template <class... ComponentTypes>
	SlimEntity* CreateEntity(ComponentTypes&&... args);

	template <class ComponentT>
	void AddComponent(SlimEntity& entity, ComponentT component);

	template <class ComponentT>
	void RemoveComponent(SlimEntity& entity);

	void RemoveComponent(SlimEntity& entity, size_t index);

private:
	std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>> m_entityStores;
};



template <class... ComponentTypes>
SlimEntity* World::CreateEntity(ComponentTypes&&... args) {
	static const ComponentScheme scheme = { typeid(ComponentTypes)... };

	auto it = m_entityStores.find(scheme);
	if (it == m_entityStores.end()) {
		auto [newIt, ignored_] = m_entityStores.insert(decltype(m_entityStores)::value_type{ scheme, std::make_unique<EntitySet>() });
		newIt->second->store.Extend<ComponentTypes...>();
		it = newIt;
	}
	it->second->store.PushBack(std::forward<ComponentTypes>(args)...);
	it->second->entities.push_back(std::make_unique<SlimEntity>(this, it->second.get(), it->second->store.Size() - 1));

	return it->second->entities.back().get();
}


template <class ComponentT>
void World::AddComponent(SlimEntity& entity, ComponentT component) {
	assert(entity.GetWorld() == this);

	// Naive implementation of the extended Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<SlimEntity>>&>(entity.GetStore()->entities);
	auto& currentStore = const_cast<EntityStore&>(entity.GetStore()->store);
	const size_t currentIndex = entity.GetIndex();
	const ComponentScheme& currentScheme = currentStore.Scheme();
	ComponentScheme extendedScheme = currentScheme;
	extendedScheme.Insert(typeid(ComponentT));

	// Find or create extended store.
	auto it = m_entityStores.find(extendedScheme);
	if (it == m_entityStores.end()) {
		auto [newIt, ignore_] = m_entityStores.insert({ extendedScheme, std::make_unique<EntitySet>() });
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
		m_entityStores.erase(currentStore.Scheme());
	}
	entity = SlimEntity(this, it->second.get(), it->second->store.Size() - 1);
}


template <class ComponentT>
void World::RemoveComponent(SlimEntity& entity) {
	assert(entity.GetWorld() == this);

	// Find the index of the first component of specified type.
	auto& currentStore = const_cast<EntityStore&>(entity.GetStore()->store);
	const ComponentScheme& currentScheme = currentStore.Scheme();

	const auto [firstOfType, lastOfType] = currentScheme.Range(typeid(ComponentT));
	const size_t indexToRemove = std::distance(currentScheme.begin(), firstOfType);
	if (firstOfType == lastOfType) {
		throw InvalidArgumentException("Entity has no such component.");
	}

	RemoveComponent(entity, indexToRemove);
}


} // namespace inl::game