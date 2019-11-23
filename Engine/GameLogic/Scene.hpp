#pragma once

#include "ComponentMatrix.hpp"

#include <experimental/generator>
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


class Scene {
	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;
		using store_iterator = std::conditional_t<Const,
												  std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>>::const_iterator,
												  std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>>::iterator>;
		using entity_iterator = std::conditional_t<Const,
												   ContiguousVector<std::unique_ptr<Entity>>::const_iterator,
												   ContiguousVector<std::unique_ptr<Entity>>::iterator>;

	public:
		generic_iterator() = default;
		generic_iterator(store_iterator store, store_iterator storeEnd, entity_iterator entity) : m_store(store), m_storeEnd(storeEnd), m_entity(entity) {}

		generic_iterator(const generic_iterator& rhs) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_store(rhs.m_store), m_storeEnd(rhs.m_storeEnd), m_entity(rhs.m_entity) {}

		Entity& operator*() const { return **m_entity; }
		Entity* operator->() const { return m_entity->get(); }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		bool operator==(const generic_iterator&) const;
		bool operator!=(const generic_iterator&) const;

	private:
		store_iterator m_store;
		store_iterator m_storeEnd;
		entity_iterator m_entity;
	};

public:
	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;

	template <class... ComponentTypes>
	Entity* CreateEntity(ComponentTypes&&... args);
	void DeleteEntity(Entity& entity);

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	Scene& operator+=(Scene&& entities);

	std::experimental::generator<std::reference_wrapper<ComponentMatrix>> GetStores(const ComponentScheme& subset);
	std::experimental::generator<std::reference_wrapper<const ComponentMatrix>> GetStores(const ComponentScheme& subset) const;

	template <class ComponentT>
	void AddComponent(Entity& entity, ComponentT&& component);
	template <class ComponentT>
	void RemoveComponent(Entity& entity);
	void RemoveComponent(Entity& entity, size_t index);

private:
	// TODO: refactor stuff
	template <class Component>
	static size_t MoveEntityExtend(EntitySet& target, EntitySet& source, size_t sourceIndex, Component&& component);
	static size_t MoveEntityPartial(EntitySet& target, EntitySet& source, size_t sourceIndex, size_t skippedComponent);
	static size_t MoveEntity(EntitySet& target, EntitySet& source, size_t sourceIndex);


	ComponentScheme GetScheme(const ComponentMatrix& matrix);
	void MoveScheme(const ComponentScheme& scheme, EntitySet&& entitySet);
	void MergeScheme(const ComponentScheme& scheme, EntitySet&& entitySet);
	void AppendScheme(EntitySet& target, EntitySet&& source);

private:
	std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>> m_componentStores;
};


} // namespace inl::game


#include "Entity.hpp"


namespace inl::game {


template <bool Const>
Scene::generic_iterator<Const>& Scene::generic_iterator<Const>::operator++() {
	++m_entity;
	if (m_entity == m_store->second->entities.end()) {
		do {
			++m_store;
		} while (m_store != m_storeEnd && m_store->second->entities.empty());
		if (m_store != m_storeEnd) {
			m_entity = m_store->second->entities.begin();
		}
	}
	return *this;
}

template <bool Const>
Scene::generic_iterator<Const>& Scene::generic_iterator<Const>::operator--() {
	if (m_store == m_storeEnd || m_entity == m_store->second->entities.begin()) {
		do {
			--m_store;
		} while (m_store->second->entities.empty());
		m_entity = m_store->second->entities.end();
	}
	--m_entity;
	return *this;
}

template <bool Const>
Scene::generic_iterator<Const> Scene::generic_iterator<Const>::operator++(int) {
	auto cpy = *this;
	++*this;
	return cpy;
}

template <bool Const>
Scene::generic_iterator<Const> Scene::generic_iterator<Const>::operator--(int) {
	auto cpy = *this;
	--*this;
	return cpy;
}

template <bool Const>
bool Scene::generic_iterator<Const>::operator==(const generic_iterator& rhs) const {
	if (m_store == m_storeEnd) {
		return m_store == rhs.m_store;
	}
	return m_store == rhs.m_store && m_entity == rhs.m_entity;
}

template <bool Const>
bool Scene::generic_iterator<Const>::operator!=(const generic_iterator& rhs) const {
	return !(*this == rhs);
}


template <class... ComponentTypes>
Entity* Scene::CreateEntity(ComponentTypes&&... args) {
	static const ComponentScheme scheme = { typeid(ComponentTypes)... };

	auto it = m_componentStores.find(scheme);
	if (it == m_componentStores.end()) {
		auto [newIt, ignored_] = m_componentStores.insert(decltype(m_componentStores)::value_type{ scheme, std::make_unique<EntitySet>() });
		if constexpr (sizeof...(ComponentTypes) > 0) {
			(newIt->second->store.types.push_back(_ComponentVector<ComponentTypes>{}), ...);
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
void Scene::AddComponent(Entity& entity, ComponentT&& component) {
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
void Scene::RemoveComponent(Entity& entity) {
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


} // namespace inl::game