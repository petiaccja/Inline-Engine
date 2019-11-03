#include "ComponentMatrix.hpp"

namespace inl::game {

//------------------------------------------------------------------------------
// EntityVector -- implementation
//------------------------------------------------------------------------------

EntityVector::reference EntityVector::operator[](size_t index) {
	return *(begin() + index);
}

EntityVector::const_reference EntityVector::operator[](size_t index) const {
	return *(begin() + index);
}

EntityVector::iterator EntityVector::begin() {
	return iterator{ 0, m_matrix };
}

EntityVector::iterator EntityVector::end() {
	return iterator{ size(), m_matrix };
}

EntityVector::const_iterator EntityVector::begin() const {
	return const_iterator{ 0, m_matrix };
}

EntityVector::const_iterator EntityVector::end() const {
	return const_iterator{ size(), m_matrix };
}

EntityVector::const_iterator EntityVector::cbegin() const {
	return const_iterator{ 0, m_matrix };
}

EntityVector::const_iterator EntityVector::cend() const {
	return const_iterator{ size(), m_matrix };
}

EntityVector::size_type EntityVector::size() const {
	return !m_matrix.empty() ? m_matrix[0]->Size() : 0;
}

void EntityVector::insert(const_iterator where, const_reference entity) {
	if (m_matrix.empty()) {
		throw InvalidStateException("Cannot add an entity when there are no components.");
	}
	auto index = where.get_index();
	for (auto& vector : m_matrix) {
		vector->InsertDefault(index);
	}
	*(begin() + index) = entity;
}

void EntityVector::erase(const_iterator where) {
	auto index = where.get_index();
	for (auto& vector : m_matrix) {
		vector->Erase(index);
	}
}


//------------------------------------------------------------------------------
// TypeVector -- implementation
//------------------------------------------------------------------------------

TypeVector::reference TypeVector::operator[](size_t index) {
	return *(begin() + index);
}

TypeVector::const_reference TypeVector::operator[](size_t index) const {
	return *(begin() + index);
}

TypeVector::iterator TypeVector::begin() {
	return iterator{ 0, m_matrix };
}

TypeVector::iterator TypeVector::end() {
	return iterator{ size(), m_matrix };
}

TypeVector::const_iterator TypeVector::begin() const {
	return const_iterator{ 0, m_matrix };
}

TypeVector::const_iterator TypeVector::end() const {
	return const_iterator{ size(), m_matrix };
}

TypeVector::const_iterator TypeVector::cbegin() const {
	return const_iterator{ 0, m_matrix };
}

TypeVector::const_iterator TypeVector::cend() const {
	return const_iterator{ size(), m_matrix };
}

TypeVector::size_type TypeVector::size() const {
	return m_matrix.size();
}

void TypeVector::erase(const_iterator where) {
	m_matrix.erase(m_matrix.begin() + where.get_index());
	recompute_order();
}

void TypeVector::recompute_order() {
	std::vector<std::pair<std::type_index, size_t>> orderedTypes;
	m_orderedTypes.reserve(m_matrix.size());
	size_t idx = 0;
	for (auto& vec : m_matrix) {
		orderedTypes.push_back({ vec->Type(), idx++ });
	}
	std::stable_sort(orderedTypes.begin(), orderedTypes.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});
	m_orderedTypes = std::move(orderedTypes);
}


} // namespace inl::game