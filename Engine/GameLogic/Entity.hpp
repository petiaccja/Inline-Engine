#pragma once

#include "ComponentMatrix.hpp"

#include <cassert>


namespace inl::game {


class World;
struct EntitySet;
class ComponentScheme;


namespace impl {
	template <class ComponentT>
	class ComponentIterator {
		static constexpr bool isConst = std::is_const_v<std::remove_reference_t<ComponentT>>;



		std::conditional<isConst, const ComponentMatrix*, ComponentMatrix*> m_store;
		size_t m_entityIndex; // At which index in the vectors this entity's components reside.
		size_t m_first; // Index of the first component vector with ComponentT.
		size_t m_last; // Index of the past-the-last component vector with ComponentT.
	};
} // namespace impl



class Entity {
public:
	Entity() = default;
	Entity(World* world, EntitySet* store, size_t index) : m_world(world), m_store(store), m_index(index) {}

	template <class ComponentT>
	void AddComponent(ComponentT&& component);

	template <class ComponentT>
	void RemoveComponent();

	void RemoveComponent(size_t index);

	template <class ComponentT>
	bool HasComponent() const;

	template <class ComponentT>
	ComponentT& GetFirstComponent();

	template <class ComponentT>
	const ComponentT& GetFirstComponent() const;

	const World* GetWorld() const { return m_world; }
	const EntitySet* GetStore() const { return m_store; }
	size_t GetIndex() const { return m_index; }

private:
	World* m_world;
	EntitySet* m_store;
	size_t m_index;
};

} // namespace inl::game



#include "World.hpp"


namespace inl::game {

template <class ComponentT>
void Entity::AddComponent(ComponentT&& component) {
	assert(m_world);
	m_world->AddComponent(*this, std::forward<ComponentT>(component));
}


template <class ComponentT>
void Entity::RemoveComponent() {
	assert(m_world);
	m_world->RemoveComponent<ComponentT>(*this);
}

template <class ComponentT>
bool Entity::HasComponent() const {
	assert(m_store);
	auto& store = m_store->store;
	auto [first, last] = store.types.equal_range(typeid(ComponentT));
	return first < last;
}

template <class ComponentT>
ComponentT& Entity::GetFirstComponent() {
	assert(m_store);
	auto& store = m_store->store;
	auto [first, last] = store.types.equal_range(typeid(ComponentT));
	if (first == last) {
		throw InvalidArgumentException("No such component in entity.");
	}
	return m_store->store.entities[m_index].get<ComponentT>(first->second);
}


template <class ComponentT>
const ComponentT& Entity::GetFirstComponent() const {
	assert(m_store);
	auto& store = m_store->store;
	auto [first, last] = store.types.equal_range(typeid(ComponentT));
	if (first == last) {
		throw InvalidArgumentException("No such component in entity.");
	}
	return m_store->store.entities[m_index].get<ComponentT>(first->second);
}


} // namespace inl::game