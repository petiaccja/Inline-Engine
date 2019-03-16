#include "ComponentScheme.hpp"

#include "BaseLibrary/HashCombine.hpp"
#include "BaseLibrary/Range.hpp"

#include <algorithm>
#include <ppltasks.h>


namespace inl::game {


void ComponentScheme::Insert(std::type_index type) {
	auto last = std::upper_bound(begin(), end(), type);
	m_types.insert(last, type);
}


void ComponentScheme::Erase(const_iterator it) {
	m_types.erase(it);
}


std::pair<ComponentScheme::const_iterator, ComponentScheme::const_iterator> ComponentScheme::Range(std::type_index type) const {
	return std::equal_range(begin(), end(), type);
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
