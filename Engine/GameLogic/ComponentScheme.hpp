#pragma once


#include <typeindex>
#include <vector>
#include <unordered_set>
#include <initializer_list>


namespace inl::game {


class ComponentScheme {
public:
	ComponentScheme();
	ComponentScheme(std::initializer_list<std::type_index> types);
	ComponentScheme(const ComponentScheme&) = default;
	ComponentScheme(ComponentScheme&&) = default;
	ComponentScheme& operator=(const ComponentScheme&) = default;
	ComponentScheme& operator=(ComponentScheme&&) = default;

	using const_iterator = std::vector<std::type_index>::const_iterator;

	const_iterator Insert(std::type_index type);
	void Erase(const_iterator it);
	std::pair<const_iterator, const_iterator> Range(std::type_index type) const;
	std::pair<size_t, size_t> Index(std::type_index type) const;
	size_t Size() const;

	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	size_t GetHashCode() const;

	friend bool operator==(const ComponentScheme&, const ComponentScheme&);
	friend bool operator!=(const ComponentScheme&, const ComponentScheme&);

private:
	void Rehash();

private:
	std::vector<std::type_index> m_types;
	size_t m_hash;
};


} // namespace inl::game


namespace std {

template <>
struct hash<inl::game::ComponentScheme> {
	size_t operator()(const inl::game::ComponentScheme& obj) const noexcept {
		return obj.GetHashCode();
	}
};


} // namespace std