#pragma once

#include "ComponentMatrix.hpp"
#include "ComponentScheme.hpp"

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
	ComponentMatrix matrix;
};


class Scene {
	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;
		using set_iterator = std::conditional_t<Const,
												  std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>>::const_iterator,
												  std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>>::iterator>;
		using entity_iterator = std::conditional_t<Const,
												   ContiguousVector<std::unique_ptr<Entity>>::const_iterator,
												   ContiguousVector<std::unique_ptr<Entity>>::iterator>;

	public:
		generic_iterator() = default;
		generic_iterator(set_iterator setIt, set_iterator setEnd, entity_iterator entity) : m_setIt(setIt), m_setEnd(setEnd), m_entityIt(entity) {}

		generic_iterator(const generic_iterator& rhs) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_setIt(rhs.m_setIt), m_setEnd(rhs.m_setEnd), m_entityIt(rhs.m_entityIt) {}

		Entity& operator*() const { return **m_entityIt; }
		Entity* operator->() const { return m_entityIt->get(); }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		bool operator==(const generic_iterator&) const;
		bool operator!=(const generic_iterator&) const;

	private:
		set_iterator m_setIt;
		set_iterator m_setEnd;
		entity_iterator m_entityIt;
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

	std::experimental::generator<std::reference_wrapper<ComponentMatrix>> GetMatrices(const ComponentScheme& subset);
	std::experimental::generator<std::reference_wrapper<const ComponentMatrix>> GetMatrices(const ComponentScheme& subset) const;

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
	std::unordered_map<ComponentScheme, std::unique_ptr<EntitySet>> m_componentSets;
};


} // namespace inl::game


#include "Entity.hpp"


namespace inl::game {


template <bool Const>
Scene::generic_iterator<Const>& Scene::generic_iterator<Const>::operator++() {
	++m_entityIt;
	if (m_entityIt == m_setIt->second->entities.end()) {
		do {
			++m_setIt;
		} while (m_setIt != m_setEnd && m_setIt->second->entities.empty());
		if (m_setIt != m_setEnd) {
			m_entityIt = m_setIt->second->entities.begin();
		}
	}
	return *this;
}

template <bool Const>
Scene::generic_iterator<Const>& Scene::generic_iterator<Const>::operator--() {
	if (m_setIt == m_setEnd || m_entityIt == m_setIt->second->entities.begin()) {
		do {
			--m_setIt;
		} while (m_setIt->second->entities.empty());
		m_entityIt = m_setIt->second->entities.end();
	}
	--m_entityIt;
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
	if (m_setIt == m_setEnd) {
		return m_setIt == rhs.m_setIt;
	}
	return m_setIt == rhs.m_setIt && m_entityIt == rhs.m_entityIt;
}

template <bool Const>
bool Scene::generic_iterator<Const>::operator!=(const generic_iterator& rhs) const {
	return !(*this == rhs);
}


template <class... ComponentTypes>
Entity* Scene::CreateEntity(ComponentTypes&&... args) {
	static const ComponentScheme scheme = { typeid(ComponentTypes)... };

	auto it = m_componentSets.find(scheme);
	if (it == m_componentSets.end()) {
		auto [newIt, ignored_] = m_componentSets.insert(decltype(m_componentSets)::value_type{ scheme, std::make_unique<EntitySet>() });
		if constexpr (sizeof...(ComponentTypes) > 0) {
			(newIt->second->matrix.types.push_back(_ComponentVector<ComponentTypes>{}), ...);
		}
		it = newIt;
	}
	if constexpr (sizeof...(ComponentTypes) > 0) {
		it->second->matrix.entities.emplace_back(std::forward<ComponentTypes>(args)...);
	}
	it->second->entities.push_back(std::make_unique<Entity>(this, it->second.get(), it->second->entities.size()));

	return it->second->entities.back().get();
}


template <class ComponentT>
void Scene::AddComponent(Entity& entity, ComponentT&& component) {
	assert(entity.GetScene() == this);

	// STUCK HERE
	// This query and extend scheme and hashtable shit is very slow and pretty ugly...
	// New ComponentMatrix doesn't make this part any better.
	// How to rewrite?

	// Naive implementation of the extended Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<Entity>>&>(entity.GetSet()->entities);
	auto& currentMatrix = const_cast<ComponentMatrix&>(entity.GetSet()->matrix);
	const size_t currentIndex = entity.GetIndex();
	const ComponentScheme& currentScheme = GetScheme(currentMatrix);
	ComponentScheme extendedScheme = currentScheme;
	extendedScheme.Insert(typeid(ComponentT));

	// Find or create extended matrix.
	auto it = m_componentSets.find(extendedScheme);
	if (it == m_componentSets.end()) {
		auto [newIt, ignore_] = m_componentSets.insert({ extendedScheme, std::make_unique<EntitySet>() });
		newIt->second->matrix.types = currentMatrix.types;
		newIt->second->matrix.types.push_back(_ComponentVector<ComponentT>{});
		it = newIt;
	}
	auto& newMatrix = it->second->matrix;

	// Move components.
	auto source = currentMatrix.entities[currentIndex];
	newMatrix.entities.push_back(std::move(source));
	const size_t schemeOrderedIndex = extendedScheme.Index(typeid(ComponentT)).second - 1;
	const size_t componentIndex = newMatrix.types.type_order()[schemeOrderedIndex].second;
	newMatrix.entities.back().get<ComponentT>(componentIndex) = std::forward<ComponentT>(component);
	currentMatrix.entities.erase(currentMatrix.entities.begin() + currentIndex);

	// Update entities.
	it->second->entities.push_back(std::move(currentEntities[currentIndex]));
	currentEntities.erase(currentEntities.begin() + currentIndex);
	entity = Entity(this, it->second.get(), newMatrix.entities.size() - 1);
	if (currentEntities.size() > currentIndex) {
		*currentEntities[currentIndex] = Entity(this, (EntitySet*)currentEntities[currentIndex]->GetSet(), currentIndex);
	}
}


template <class ComponentT>
void Scene::RemoveComponent(Entity& entity) {
	assert(entity.GetScene() == this);

	// Find the index of the first component of specified type.
	auto& currentMatrix = const_cast<ComponentMatrix&>(entity.GetSet()->matrix);
	const ComponentScheme& currentScheme = GetScheme(currentMatrix);

	const auto [firstOfType, lastOfType] = currentScheme.Range(typeid(ComponentT));
	const size_t indexToRemove = std::distance(currentScheme.begin(), firstOfType);
	if (firstOfType == lastOfType) {
		throw InvalidArgumentException("Entity has no such component.");
	}

	RemoveComponent(entity, indexToRemove);
}


} // namespace inl::game