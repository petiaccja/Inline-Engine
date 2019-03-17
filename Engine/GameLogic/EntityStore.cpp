#include "EntityStore.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/HashCombine.hpp>


namespace inl::game {



void EntityStore::Erase(size_t index) {
	for (auto& vec : m_components) {
		vec->Erase(index);
	}
}

size_t EntityStore::Size() const {
	return m_components.empty() ? 0 : m_components.front()->Size();
}

const ComponentScheme& EntityStore::Scheme() const {
	return m_scheme;
}


} // namespace inl::game
