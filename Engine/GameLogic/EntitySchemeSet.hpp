#pragma once

#include "ComponentMatrix.hpp"
#include "ComponentScheme.hpp"

#include <compare>
#include <memory>
#include <vector>


namespace inl::game {

class Scene;
class Entity;


class EntitySchemeSet {
	using EntityVector = ContiguousVector<std::unique_ptr<Entity>>;

	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;
		using EntityIterator = std::conditional_t<Const, EntityVector::const_iterator, EntityVector::iterator>;

	public:
		using value_type = Entity;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = Entity*;
		using iterator_category = std::random_access_iterator_tag;

		generic_iterator() = default;
		generic_iterator(EntityIterator it) : m_it(it) {}

		generic_iterator(const generic_iterator& rhs) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_it(rhs.m_it) {}

		templ::add_const_conditional<value_type, Const>& operator*() const { return **m_it; }
		templ::add_const_conditional<value_type, Const>* operator->() const { return m_it->get(); }

		generic_iterator& operator+=(size_t n);
		generic_iterator& operator-=(size_t n);
		friend generic_iterator operator+(const generic_iterator& it, size_t n) { return generic_iterator{ it } += n; }
		friend generic_iterator operator-(const generic_iterator& it, size_t n) { return generic_iterator{ it } -= n; }
		friend generic_iterator operator+(size_t n, const generic_iterator& it) { return it + n; }
		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		size_t operator-(const generic_iterator& rhs) const;
		auto operator<=>(const generic_iterator&) const = default;

	private:
		EntityIterator m_it;
	};

public:
	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;

	EntitySchemeSet(Scene& parent);

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	template <class... Components>
	Entity& Create(Components&&... components);
	void Destroy(Entity& entity);
	void Clear();
	size_t Size() const;
	bool Empty() const;

	EntitySchemeSet& operator+=(EntitySchemeSet&& rhs);

	template <class... ComponentTypes>
	void SetComponentTypes();
	template <class ComponentType>
	void AddComponent();
	void RemoveComponent(size_t index);
	void CopyComponentTypes(const EntitySchemeSet& model);

	Entity& operator[](size_t index);
	const Entity& operator[](size_t index) const;

	template <class Component>
	size_t Splice(EntitySchemeSet& source, size_t sourceIndex, Component&& component);
	size_t Splice(EntitySchemeSet& source, size_t sourceIndex, size_t skippedComponent);
	size_t Splice(EntitySchemeSet& source, size_t sourceIndex);

	Scene& GetParent();
	const Scene& GetParent() const;
	ComponentMatrix& GetMatrix();
	const ComponentMatrix& GetMatrix() const;
	const ComponentScheme& GetScheme() const;

private:
	EntityVector m_entities;
	ComponentMatrix m_components;
	Scene& m_parent;
	ComponentScheme m_scheme;
};


} // namespace inl::game


#include "Entity.hpp"


namespace inl::game {


template <bool Const>
EntitySchemeSet::generic_iterator<Const>& EntitySchemeSet::generic_iterator<Const>::operator+=(size_t n) {
	m_it += n;
	return *this;
}

template <bool Const>
EntitySchemeSet::generic_iterator<Const>& EntitySchemeSet::generic_iterator<Const>::operator-=(size_t n) {
	m_it -= n;
	return *this;
}

template <bool Const>
EntitySchemeSet::generic_iterator<Const>& EntitySchemeSet::generic_iterator<Const>::operator++() {
	++m_it;
	return *this;
}

template <bool Const>
EntitySchemeSet::generic_iterator<Const>& EntitySchemeSet::generic_iterator<Const>::operator--() {
	--m_it;
	return *this;
}

template <bool Const>
EntitySchemeSet::generic_iterator<Const> EntitySchemeSet::generic_iterator<Const>::operator++(int) {
	auto cpy = *this;
	++*this;
	return cpy;
}

template <bool Const>
EntitySchemeSet::generic_iterator<Const> EntitySchemeSet::generic_iterator<Const>::operator--(int) {
	auto cpy = *this;
	--*this;
	return cpy;
}

template <bool Const>
size_t EntitySchemeSet::generic_iterator<Const>::operator-(const generic_iterator& rhs) const {
	return m_it - rhs.m_it;
}

template <class... Components>
Entity& EntitySchemeSet::Create(Components&&... components) {
	size_t index = m_entities.size();
	m_entities.push_back(std::make_unique<Entity>(&m_parent, this, index));
	m_components.entities.emplace_back(std::forward<Components>(components)...);
	return *m_entities.back();
}

template <class... ComponentTypes>
void EntitySchemeSet::SetComponentTypes() {
	m_components.types.clear();
	(..., m_components.types.push_back(_ComponentVector<ComponentTypes>{}));
	m_components.entities.resize(m_entities.size());
	m_scheme = { typeid(ComponentTypes)... };
}

template <class ComponentType>
void EntitySchemeSet::AddComponent() {
	m_components.types.push_back(_ComponentVector<ComponentType>{});
	m_scheme.Insert(typeid(ComponentType));
}

template <class Component>
size_t EntitySchemeSet::Splice(EntitySchemeSet& source, size_t sourceIndex, Component&& component) {
	m_entities.push_back(std::move(source.m_entities[sourceIndex]));
	source.m_entities.erase(m_entities.begin() + sourceIndex);

	m_components.entities.emplace_back();
	m_components.entities.back().assign_extend(std::move(source.m_components.entities.back()), std::forward<Component>(component));
	source.m_components.entities.erase(source.m_components.entities.begin() + sourceIndex);

	m_entities.back()->m_index = m_entities.size() - 1;
	m_entities.back()->m_set = this;

	if (source.Size() > sourceIndex) {
		source.m_entities[sourceIndex]->m_index = sourceIndex;
	}

	return m_entities.size() - 1;
}


} // namespace inl::game