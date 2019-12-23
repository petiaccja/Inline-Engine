#pragma once

#include "ComponentMatrix.hpp"
#include "ComponentScheme.hpp"

#include <experimental/generator>
#include <unordered_map>


namespace inl::game {

class Entity;
class EntitySchemeSet;


class Scene {
	using ComponentSetMap = std::unordered_map<ComponentScheme, std::unique_ptr<EntitySchemeSet>>;

	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;
		using set_iterator = std::conditional_t<Const,
												ComponentSetMap::const_iterator,
												ComponentSetMap::iterator>;
	public:
		generic_iterator() = default;
		generic_iterator(set_iterator setIt, set_iterator setEnd, size_t entityIdx) : m_setIt(setIt), m_setEnd(setEnd), m_entityIdx(entityIdx) {}

		generic_iterator(const generic_iterator& rhs) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_setIt(rhs.m_setIt), m_setEnd(rhs.m_setEnd), m_entityIdx(rhs.m_entityIdx) {}

		Entity& operator*() const { return (*m_setIt->second)[m_entityIdx]; }
		Entity* operator->() const { return &(*m_setIt->second)[m_entityIdx]; }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		bool operator==(const generic_iterator&) const;
		bool operator!=(const generic_iterator&) const;

	private:
		set_iterator m_setIt;
		set_iterator m_setEnd;
		size_t m_entityIdx;
	};

public:
	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;

	template <class... ComponentTypes>
	Entity* CreateEntity(ComponentTypes&&... args);
	void DeleteEntity(Entity& entity);
	void Clear();

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	Scene& operator+=(Scene&& entities);

	std::experimental::generator<std::reference_wrapper<EntitySchemeSet>> GetSchemeSets(const ComponentScheme& subset);
	std::experimental::generator<std::reference_wrapper<const EntitySchemeSet>> GetSchemeSets(const ComponentScheme& subset) const;

	template <class ComponentT>
	void AddComponent(Entity& entity, ComponentT&& component);
	template <class ComponentT>
	void RemoveComponent(Entity& entity);
	void RemoveComponent(Entity& entity, size_t index);

private:
	ComponentScheme GetScheme(const ComponentMatrix& matrix);
	void MergeSchemeSet(EntitySchemeSet&& entitySet);

private:
	ComponentSetMap m_componentSets;
};


} // namespace inl::game


#include "Entity.hpp"
#include "EntitySchemeSet.hpp"


namespace inl::game {


template <bool Const>
Scene::generic_iterator<Const>& Scene::generic_iterator<Const>::operator++() {
	++m_entityIdx;
	if (m_entityIdx == m_setIt->second->Size()) {
		do {
			++m_setIt;
		} while (m_setIt != m_setEnd && m_setIt->second->Empty());
		if (m_setIt != m_setEnd) {
			m_entityIdx = 0;
		}
	}
	return *this;
}

template <bool Const>
Scene::generic_iterator<Const>& Scene::generic_iterator<Const>::operator--() {
	if (m_setIt == m_setEnd || m_entityIdx == 0) {
		do {
			--m_setIt;
		} while (m_setIt->second->Empty());
		m_entityIdx = m_setIt->second->Size();
	}
	--m_entityIdx;
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
	return m_setIt == rhs.m_setIt && m_entityIdx == rhs.m_entityIdx;
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
		auto [newIt, ignored_] = m_componentSets.insert(ComponentSetMap::value_type{ scheme, std::make_unique<EntitySchemeSet>(*this) });
		newIt->second->SetComponentTypes<ComponentTypes...>();
		it = newIt;
	}

	Entity& entity = it->second->Create(std::forward<ComponentTypes>(args)...);
	return &entity;
}


template <class ComponentT>
void Scene::AddComponent(Entity& entity, ComponentT&& component) {
	assert(entity.GetScene() == this);

	auto& currentSet = const_cast<EntitySchemeSet&>(*entity.GetSet());
	size_t currentIndex = entity.GetIndex();
	auto& currentScheme = currentSet.GetScheme();

	// Find extended set.
	ComponentScheme extendedScheme = currentScheme;
	extendedScheme.Insert(typeid(ComponentT));
	auto it = m_componentSets.find(extendedScheme);
	if (it == m_componentSets.end()) {
		auto [newIt, ignore_] = m_componentSets.insert({ extendedScheme, std::make_unique<EntitySchemeSet>(*this) });
		newIt->second->CopyComponentTypes(currentSet);
		newIt->second->AddComponentType<ComponentT>();
		assert(extendedScheme == newIt->second->GetScheme());
		it = newIt;
	}
	auto& newSet = it->second;

	// Splice entity.
	newSet->SpliceExtend(currentSet, currentIndex, std::forward<ComponentT>(component));
}


template <class ComponentT>
void Scene::RemoveComponent(Entity& entity) {
	assert(entity.GetScene() == this);

	// Find the index of the first component of specified type.
	const ComponentScheme& currentScheme = entity.GetSet()->GetScheme();

	const auto [firstOfType, lastOfType] = currentScheme.Range(typeid(ComponentT));
	const size_t indexToRemove = std::distance(currentScheme.begin(), firstOfType);
	if (firstOfType == lastOfType) {
		throw InvalidArgumentException("Entity has no such component.");
	}

	RemoveComponent(entity, indexToRemove);
}


} // namespace inl::game