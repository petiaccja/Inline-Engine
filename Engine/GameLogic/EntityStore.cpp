#include "EntityStore.hpp"

#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::game {


void EntityStore::SpliceBackReduce(EntityStore& other, size_t index, const std::vector<bool>& selection) {
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


void EntityStore::Reduce(size_t index) {
	assert(index < m_scheme.Size());

	m_scheme.Erase(m_scheme.begin() + index);
	m_components.erase(m_components.begin() + index);
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


EntityStore EntityStore::CloneScheme() const {
	EntityStore clone;
	clone.m_scheme = m_scheme;
	clone.m_components.reserve(m_components.size());

	for (auto& componentVector : m_components) {
		clone.m_components.push_back(componentVector->Clone());
	}

	return clone;
}


} // namespace inl::game
