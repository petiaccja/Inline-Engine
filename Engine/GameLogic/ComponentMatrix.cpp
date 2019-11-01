#include "ComponentMatrix.hpp"

namespace inl::game {


inline ComponentMatrix::Iterator ComponentMatrix::InsertEntity(ConstIterator where) {
	for (auto& container : m_componentVectors) {
		container->InsertDefault(where.m_ref.m_index);
	}
	return Iterator{ *this, where.m_ref.m_index };
}

inline void ComponentMatrix::PushBackEntity() {
	InsertEntity(EndEntity());
}

inline void ComponentMatrix::EraseEntity(ConstIterator where) {
	for (auto& container : m_componentVectors) {
		container->Erase(where.m_ref.m_index);
	}
}

inline void ComponentMatrix::EraseEntity(ConstIterator first, ConstIterator last) {
	for (auto& container : m_componentVectors) {
		container->Erase(first.m_ref.m_index, last.m_ref.m_index);
	}
}


ComponentMatrix::Iterator ComponentMatrix::BeginEntity() {
	return { *this, 0 };
}

ComponentMatrix::Iterator ComponentMatrix::EndEntity() {
	return { *this, SizeEntity() };
}

ComponentMatrix::ConstIterator ComponentMatrix::BeginEntity() const {
	return { *this, 0 };
}

ComponentMatrix::ConstIterator ComponentMatrix::EndEntity() const {
	return { *this, SizeEntity() };
}

ComponentMatrix::ConstIterator ComponentMatrix::CBeginEntity() const {
	return BeginEntity();
}

ComponentMatrix::ConstIterator ComponentMatrix::CEndEntity() const {
	return EndEntity();
}

size_t ComponentMatrix::SizeEntity() const {
	// All should be the same size!
	return m_componentVectors.empty() ? 0 : m_componentVectors[0]->Size();
}


} // namespace inl::game