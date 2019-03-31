#include "EntityStore.hpp"

#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::game {


void EntityStore::SpliceBack(EntityStore& other, const std::vector<bool>& selection, size_t index) {
	assert(selection.size() == other.m_components.size());

	size_t myVectorIndex = 0;
	size_t otherVectorIndex = 0;
	for (bool selected : selection) {
		if (selected) {
			m_components[myVectorIndex]->SpliceBack(*other.m_components[otherVectorIndex], index);
			assert(myVectorIndex < m_components.size());
			++myVectorIndex;
		}
		else {
			other.m_components[otherVectorIndex]->Erase(index);
		}
		++otherVectorIndex;
	}
	assert(myVectorIndex == Scheme().Size());
}


void EntityStore::Erase(size_t index) {
	for (auto& vec : m_components) {
		vec->Erase(index);
	}
}


size_t EntityStore::Size() const {
	// Just for debugging.
	auto AllSame = [this] {
		bool same = true;
		size_t size = m_components.empty() ? 0 : m_components.front()->Size();
		for (auto& cv : m_components) {
			same = same && cv->Size() == size;
		}
		return same;
	};
	assert(AllSame());

	// Actual code.
	return m_components.empty() ? 0 : m_components.front()->Size();
}


const ComponentScheme& EntityStore::Scheme() const {
	return m_scheme;
}


} // namespace inl::game
