#include "ComponentScheme.hpp"

#include "BaseLibrary/HashCombine.hpp"
#include "BaseLibrary/Range.hpp"

#include <algorithm>


namespace inl::game {


ComponentScheme::ComponentScheme() {
	Rehash();
}

ComponentScheme::ComponentScheme(std::initializer_list<std::type_index> types) : m_types(types) {
	std::sort(m_types.begin(), m_types.end());
	Rehash();
}

auto ComponentScheme::Insert(std::type_index type) -> const_iterator {
	auto last = std::upper_bound(begin(), end(), type);
	auto index = last - begin();
	m_types.insert(last, type);
	Rehash();
	return begin() + index; // Stupid recalculation of the iterator from index because insert might invalidated it.
}


void ComponentScheme::Erase(const_iterator it) {
	m_types.erase(it);
	Rehash();
}


std::pair<ComponentScheme::const_iterator, ComponentScheme::const_iterator> ComponentScheme::Range(std::type_index type) const {
	return std::equal_range(begin(), end(), type);
}


std::pair<size_t, size_t> ComponentScheme::Index(std::type_index type) const {
	auto [first, last] = Range(type);
	auto beg = begin();
	return { std::distance(beg, first), std::distance(beg, last) };
}


size_t ComponentScheme::Size() const {
	return m_types.size();
}

bool ComponentScheme::Empty() const {
	return Size() == 0;
}


bool ComponentScheme::SubsetOf(const ComponentScheme& superset) const {
	auto curIt = this->cbegin();
	auto supIt = superset.cbegin();
	auto curEnd = this->cend();
	auto supEnd = superset.cend();
	while (curIt != curEnd && supIt != supEnd) {
		if (*curIt == *supIt) {
			++curIt;
			++supIt;
		}
		else if (*curIt > *supIt) {
			++supIt;
		}
		else {
			return false;
		}
	}
	return curIt == curEnd;
}


ComponentScheme::const_iterator ComponentScheme::begin() const {
	return m_types.begin();
}


ComponentScheme::const_iterator ComponentScheme::end() const {
	return m_types.end();
}


ComponentScheme::const_iterator ComponentScheme::cbegin() const {
	return m_types.cbegin();
}


ComponentScheme::const_iterator ComponentScheme::cend() const {
	return m_types.cend();
}


size_t ComponentScheme::GetHashCode() const {
	return m_hash;
}


void ComponentScheme::Rehash() {
	m_hash = 0;
	for (const auto& t : m_types) {
		m_hash = CombineHash(m_hash, t.hash_code());
	}
}


bool operator==(const ComponentScheme& lhs, const ComponentScheme& rhs) {
	if (lhs.m_types.size() != rhs.m_types.size()) {
		return false;
	}
	for (auto i : Range(lhs.m_types.size())) {
		if (lhs.m_types[i] != rhs.m_types[i]) {
			return false;
		}
	}
	return true;
}

bool operator!=(const ComponentScheme& lhs, const ComponentScheme& rhs) {
	if (lhs.m_types.size() != rhs.m_types.size()) {
		return true;
	}
	for (auto i : Range(lhs.m_types.size())) {
		if (lhs.m_types[i] != rhs.m_types[i]) {
			return true;
		}
	}
	return false;
}

} // namespace inl::game
