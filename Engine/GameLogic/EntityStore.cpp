#include "EntityStore.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/HashCombine.hpp>


namespace inl::game {


void EntityStore::SpliceBack(EntityStore& other, size_t index, const std::vector<bool>& selection) {
	assert(selection.size() == other.m_components.size());

	size_t myVectorIndex = 0;
	size_t otherVectorIndex = 0;
	for (bool selected : selection) {
		if (selected) {
			m_components[myVectorIndex]->SpliceBack(*other.m_components[otherVectorIndex], index);
			assert(myVectorIndex < m_components.size());
			++myVectorIndex;
		}
		++otherVectorIndex;
	}
}

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
