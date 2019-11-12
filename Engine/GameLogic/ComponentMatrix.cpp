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

EntityVector::reference EntityVector::back() {
	assert(!empty());
	return *--end();
}

EntityVector::reference EntityVector::front() {
	assert(!empty());
	return *begin();
}

EntityVector::const_reference EntityVector::back() const {
	assert(!empty());
	return *--cend();
}

EntityVector::const_reference EntityVector::front() const {
	assert(!empty());
	return *cbegin();
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

bool EntityVector::empty() const {
	return size() == 0;
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

TypeVector& TypeVector::operator=(const TypeVector& rhs) {
	size_t sz = size();

	auto myIt = m_orderedTypes.begin();
	auto rhsIt = rhs.m_orderedTypes.begin();
	auto myEnd = m_orderedTypes.end();
	auto rhsEnd = rhs.m_orderedTypes.end();

	std::vector<std::unique_ptr<ComponentVectorBase>> addedVectors;
	std::vector<size_t> deletedVectors;

	while (rhsIt != rhsEnd) {
		if (myIt == myEnd || rhsIt->first < myIt->first) {
			// clone & add
			addedVectors.push_back(rhs.m_matrix[rhsIt->second]->CloneEmpty());
			addedVectors.back()->Resize(sz);
			++rhsIt;
		}
		else if (rhsIt->first > myIt->first) {
			// delete my
			m_matrix[myIt->second].reset();
			++myIt;
		}
		else {
			++myIt;
			++rhsIt;
		}
	}
	while (myIt != myEnd) {
		m_matrix[myIt->second].reset();
		++myIt;
	}
	

	m_matrix.reserve(rhs.m_matrix.size());
	const auto lastKept = std::remove_if(m_matrix.begin(), m_matrix.end(), [](const std::unique_ptr<ComponentVectorBase>& arg) { return !arg; });
	m_matrix.erase(lastKept, m_matrix.end());
	for (auto& added : addedVectors) {
		m_matrix.push_back(std::move(added));
	}

	recompute_order();
	return *this;
}

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

auto TypeVector::equal_range(std::type_index type) const
	-> std::pair<std::vector<std::pair<std::type_index, size_t>>::const_iterator, std::vector<std::pair<std::type_index, size_t>>::const_iterator> {
	struct Compare {
		bool operator()(const std::type_index& lhs, const std::pair<std::type_index, size_t>& rhs) const {
			return lhs < rhs.first;
		}
		bool operator()(const std::pair<std::type_index, size_t>& lhs, const std::type_index& rhs) const {
			return lhs.first < rhs;
		}
		bool operator()(const std::pair<std::type_index, size_t>& lhs, const std::pair<std::type_index, size_t>& rhs) const {
			return lhs.first < rhs.first;
		}
		bool operator()(const std::type_index& lhs, const std::type_index& rhs) const {
			return lhs < rhs;
		}
	};
	return std::equal_range(type_order().begin(), type_order().end(), type, Compare{});
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