#include "Entity.hpp"

#include "World.hpp"

namespace inl::game {


void Entity::RemoveComponent(size_t index) {
	assert(m_world);
	m_world->RemoveComponent(*this, index);
}


const ComponentScheme& Entity::GetScheme() const {
	assert(m_store);
	return m_store->store.Scheme();
}


} // namespace inl::game