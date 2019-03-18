#include "Entity.hpp"


namespace inl::game {


Entity::Entity(Entity&& rhs) noexcept {
	std::swap(m_components, rhs.m_components);
	for (auto& v : m_components) {
		v.second->m_entity = this;
	}
	rhs.m_components.clear();
}

Entity& Entity::operator=(Entity&& rhs) noexcept {
	// Swap two operands.
	std::swap(m_components, rhs.m_components);
	for (auto& v : m_components) {
		v.second->m_entity = this;
	}
	for (auto& v : rhs.m_components) {
		v.second->m_entity = &rhs;
	}
	return *this;
}

Entity::~Entity() {
	for (auto& v : m_components) {
		v.second->m_entity = nullptr;
	}
}


} // namespace inl::game