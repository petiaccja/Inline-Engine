#include "GameEntity.hpp"


namespace inl::game {


GameEntity::GameEntity(GameEntity&& rhs) noexcept {
	std::swap(m_components, rhs.m_components);
	for (auto& v : m_components) {
		v.second->m_entity = this;
	}
	rhs.m_components.clear();
}

GameEntity& GameEntity::operator=(GameEntity&& rhs) noexcept {
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

GameEntity::~GameEntity() {
	for (auto& v : m_components) {
		v.second->m_entity = nullptr;
	}
}


} // namespace inl::game