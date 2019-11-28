#include "Exception/Exception.hpp"

#include <any>
#include <typeindex>
#include <unordered_map>


namespace inl {


class DynamicTuple {
public:
	template <class... Members>
	DynamicTuple(Members&&... members);
	DynamicTuple(const DynamicTuple&) = default;
	DynamicTuple(DynamicTuple&&) = default;

	DynamicTuple& operator=(const DynamicTuple&) = default;
	DynamicTuple& operator=(DynamicTuple&&) = default;

	template <class T>
	void Insert(T&& obj);

	template <class T>
	void Erase();

	void Clear();

	template <class T>
	T& Get();

	template <class T>
	const T& Get() const;

	template <class T>
	bool Has() const;

private:
	std::unordered_map<std::type_index, std::any> m_members;
};


template <class... Members>
DynamicTuple::DynamicTuple(Members&&... members) {
	(..., Insert(std::forward<Members>(members)));
}

template <class T>
void DynamicTuple::Insert(T&& obj) {
	if (Has<T>()) {
		throw InvalidArgumentException("Can only contain one instance of a specific type.");
	}
	m_members[typeid(T)] = std::forward<T>(obj);
}

template <class T>
void DynamicTuple::Erase() {
	auto it = m_members.find(typeid(T));
	if (it != m_members.end()) {
		m_members.erase(it);
	}
}

inline void DynamicTuple::Clear() {
	m_members.clear();
}

template <class T>
T& DynamicTuple::Get() {
	auto it = m_members.find(typeid(T));
	if (it != m_members.end()) {
		return std::any_cast<T&>(it->second);
	}
	throw OutOfRangeException("Does not contain this member.");
}

template <class T>
const T& DynamicTuple::Get() const {
	auto it = m_members.find(typeid(T));
	if (it != m_members.end()) {
		return std::any_cast<const T&>(it->second);
	}
	throw OutOfRangeException("Does not contain this member.");
}

template <class T>
bool DynamicTuple::Has() const {
	auto it = m_members.find(typeid(T));
	return it != m_members.end();
}


} // namespace inl