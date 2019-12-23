#pragma once

#include "ComponentMatrix.hpp"

#include <cassert>


namespace inl::game {


class Scene;
class EntitySchemeSet;
class ComponentScheme;



class Entity {
	friend class EntitySchemeSet;

public:
	Entity() = default;
	Entity(Scene* scene, EntitySchemeSet* set, size_t index) : m_scene(scene), m_set(set), m_index(index) {}
	Entity(const Entity&) = delete;
	Entity(Entity&&) = delete;
	Entity& operator=(const Entity&) = delete;
	Entity& operator=(Entity&&) = delete;

	template <class ComponentT>
	void AddComponent(ComponentT&& component);

	template <class ComponentT>
	void RemoveComponent();

	void RemoveComponent(size_t index);

	template <class ComponentT>
	bool HasComponent() const;

	template <class ComponentT>
	decltype(auto) GetFirstComponent();

	template <class ComponentT>
	decltype(auto) GetFirstComponent() const;

	const Scene* GetScene() const { return m_scene; }
	const EntitySchemeSet* GetSet() const { return m_set; }
	size_t GetIndex() const { return m_index; }

private:
	Scene* m_scene;
	EntitySchemeSet* m_set;
	size_t m_index;
};

} // namespace inl::game



#include "EntitySchemeSet.hpp"
#include "Scene.hpp"


namespace inl::game {

template <class ComponentT>
void Entity::AddComponent(ComponentT&& component) {
	assert(m_scene);
	m_scene->AddComponent(*this, std::forward<ComponentT>(component));
}


template <class ComponentT>
void Entity::RemoveComponent() {
	assert(m_scene);
	m_scene->RemoveComponent<ComponentT>(*this);
}

template <class ComponentT>
bool Entity::HasComponent() const {
	assert(m_set);
	auto& matrix = m_set->GetMatrix();
	auto [first, last] = matrix.types.equal_range(typeid(ComponentT));
	return first < last;
}

template <class ComponentT>
decltype(auto) Entity::GetFirstComponent() {
	assert(m_set);
	auto& matrix = m_set->GetMatrix();
	auto [first, last] = matrix.types.equal_range(typeid(ComponentT));
	if (first == last) {
		throw InvalidArgumentException("No such component in entity.");
	}
	return m_set->GetMatrix().entities[m_index].get<ComponentT>(first->second);
}


template <class ComponentT>
decltype(auto) Entity::GetFirstComponent() const {
	assert(m_set);
	auto& matrix = m_set->GetMatrix();
	auto [first, last] = matrix.types.equal_range(typeid(ComponentT));
	if (first == last) {
		throw InvalidArgumentException("No such component in entity.");
	}
	return m_set->GetMatrix().entities[m_index].get<ComponentT>(first->second);
}


} // namespace inl::game