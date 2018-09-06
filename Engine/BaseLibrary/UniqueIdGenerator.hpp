#pragma once

#include <cstdint>
#include <unordered_map>

namespace inl {


class UniqueId {
	friend struct std::hash<UniqueId>;
public:
	explicit UniqueId(uint64_t value = ~0ull) : value(value) {}
	bool operator==(const UniqueId& rhs) const { return value == rhs.value; }
	bool operator!=(const UniqueId& rhs) const { return value != rhs.value; }
	bool operator< (const UniqueId& rhs) const { return value < rhs.value; }
	bool operator> (const UniqueId& rhs) const { return value > rhs.value; }
	bool operator<=(const UniqueId& rhs) const { return value <= rhs.value; }
	bool operator>=(const UniqueId& rhs) const { return value >= rhs.value; }

	uint64_t Value() const { return value; }
	uint64_t Hash() const { return std::hash<uint64_t>()(value); }
private:
	uint64_t value;
};



/// <summary> This class can be used to speed up hashing and comparison of complex classes. </summary>
/// <remarks> The class used must be hashable and comparable to begin with. If these operations are
///		expensive, this class can provide a lightweight ID for each instance. For two instances,
///		their ID equals if and only if the instances themselves compare equal. The ID is lightweight,
///		fast to hash and compare. </remarks>
template <class T, class Hash = std::hash<T>, class Equal = std::equal_to<T>>
class UniqueIdGenerator {
public:
	/// <summary> Generate a unique identifier for the object. </summary>
	UniqueId operator()(const T& object);

	void Reset();

	size_t DataBaseSize() const;
private:
	std::unordered_map<T, uint64_t, Hash, Equal> m_knownObjects;
	uint64_t m_counter = 0;
};


template <class T, class Hash, class Equal>
UniqueId UniqueIdGenerator<T, Hash, Equal>::operator()(const T& object) {
	auto it = m_knownObjects.find(object);
	if (it != m_knownObjects.end()) {
		return UniqueId(it->second);
	}
	else {
		++m_counter;
		m_knownObjects[object] = m_counter;
		return UniqueId(m_counter);
	}
}


template <class T, class Hash, class Equal>
void UniqueIdGenerator<T, Hash, Equal>::Reset() {
	m_knownObjects.clear();
	m_counter = 0;
}

template<class T, class Hash, class Equal>
size_t UniqueIdGenerator<T, Hash, Equal>::DataBaseSize() const {
	return m_knownObjects.size();
}


} // namespace inl



namespace std {

template <>
struct std::hash<inl::UniqueId> {
	size_t operator()(const inl::UniqueId& obj) const {
		return std::hash<size_t>()(obj.value);
	}
};

} // namespace std