#pragma once

#include "ComponentMatrix.hpp"

#include <map>
#include <type_traits>
#include <vector>


namespace inl::game {


namespace impl {
	template <class T, bool Const>
	using AddConstOptT = std::conditional_t<Const, const T, T>;

	template <class T>
	using ComponentVectorT = ContiguousVector<std::decay_t<T>>;

	template <class T>
	using ComponentVectorOptConstT = AddConstOptT<ComponentVectorT<T>, std::is_const_v<std::remove_reference_t<T>>>;
} // namespace impl


template <bool Const, class... ComponentTypes>
class ComponentRangeIterator {
	using ComponentVectorTupleT = std::tuple<impl::ComponentVectorOptConstT<ComponentTypes>&...>;
	using ComponentValueTupleT = std::tuple<std::decay_t<ComponentTypes>...>;
	using ComponentRefTupleT = std::tuple<impl::AddConstOptT<ComponentTypes, Const>&...>;
	using ComponentRRefTupleT = std::tuple<impl::AddConstOptT<ComponentTypes, Const>&&...>;
	using ComponentPtrTupleT = std::tuple<impl::AddConstOptT<ComponentTypes, Const>*...>;

public:
	ComponentRangeIterator() = default;
	ComponentRangeIterator(ComponentVectorTupleT* componentVectors, size_t componentIndex)
		: m_componentVectors(componentVectors), m_componentIndex(componentIndex) {}

	ComponentRefTupleT operator*() { return Get(std::make_index_sequence<sizeof...(ComponentTypes)>{}); }

	// Input
	using value_type = ComponentValueTupleT;
	using reference = ComponentRefTupleT;
	using pointer = ComponentPtrTupleT;
	using rvalue_reference = ComponentRRefTupleT;
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = ptrdiff_t;
	using size_type = size_t;

	// Forward
	ComponentRangeIterator& operator++() {
		++m_componentIndex;
		return *this;
	}
	ComponentRangeIterator& operator++(int) { return ++ComponentRangeIterator(this); }

	// Bidirectional
	ComponentRangeIterator& operator--() {
		--m_componentIndex;
		return *this;
	}
	ComponentRangeIterator& operator--(int) { return --ComponentRangeIterator(this); }

	// Random access
	ComponentRangeIterator& operator+=(size_t n) {
		m_componentIndex += n;
		return *this;
	}
	ComponentRangeIterator& operator-=(size_t n) {
		m_componentIndex -= n;
		return *this;
	}
	friend ComponentRangeIterator operator+(ComponentRangeIterator it, size_t n) { return it += n; }
	friend ComponentRangeIterator operator-(ComponentRangeIterator it, size_t n) { return it -= n; }
	friend ComponentRangeIterator operator+(size_t n, ComponentRangeIterator it) { return it + n; }
	friend ptrdiff_t operator-(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return (ptrdiff_t)a.m_componentIndex - (ptrdiff_t)b.m_componentIndex; }

	friend bool operator<(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return a.m_componentIndex < b.m_componentIndex; }
	friend bool operator>(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return a.m_componentIndex > b.m_componentIndex; }
	friend bool operator<=(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return a.m_componentIndex <= b.m_componentIndex; }
	friend bool operator>=(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return a.m_componentIndex >= b.m_componentIndex; }
	friend bool operator==(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return a.m_componentIndex == b.m_componentIndex; }
	friend bool operator!=(const ComponentRangeIterator& a, const ComponentRangeIterator& b) { return a.m_componentIndex != b.m_componentIndex; }

private:
	template <size_t... Indices>
	ComponentRefTupleT Get(std::index_sequence<Indices...>) { return { std::get<Indices>(*m_componentVectors)[m_componentIndex]... }; }

private:
	ComponentVectorTupleT* m_componentVectors = 0;
	size_t m_componentIndex = 0;
};


template <class... ComponentTypes>
class ComponentRange {
public:
	using iterator = ComponentRangeIterator<false, ComponentTypes...>;
	using const_iterator = ComponentRangeIterator<true, ComponentTypes...>;

private:
	static constexpr bool IsAllConst = std::conjunction_v<std::is_const<std::remove_reference_t<ComponentTypes>>...>;
	using ComponentMatrixOptConstT = impl::AddConstOptT<ComponentMatrix, IsAllConst>;
	using ComponentVectorTupleT = std::tuple<impl::ComponentVectorOptConstT<ComponentTypes>&...>;

public:
	ComponentRange(ComponentMatrixOptConstT& componentMatrix);
	ComponentRange(ComponentMatrixOptConstT& componentMatrix, const std::vector<size_t>& componentVectorIndices);

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

private:
	std::vector<size_t> DefaultIndices(ComponentMatrixOptConstT& componentMatrix);

	template <size_t... Indices>
	ComponentVectorTupleT FindComponentVectors(ComponentMatrixOptConstT& componentMatrix, const std::vector<size_t>& componentVectorIndices, std::index_sequence<Indices...>);

private:
	ComponentVectorTupleT m_componentVectors;
};


template <class... ComponentTypes>
std::vector<size_t> ComponentRange<ComponentTypes...>::DefaultIndices(ComponentMatrixOptConstT& componentMatrix) {
	std::map<std::type_index, size_t> indexCounter;
	std::array types = { std::type_index(typeid(ComponentTypes))... };
	std::vector<size_t> indices;

	for (auto& type : types) {
		auto it = indexCounter.find(type);
		if (it == indexCounter.end()) {
			auto [first, last] = componentMatrix.types.equal_range(type);
			if (first == last) {
				throw InvalidArgumentException("Component type not found in matrix.");
			}
			indexCounter[type] = first->second;
			indices.push_back(first->second);
		}
		else {
			indices.push_back(++it->second);
		}
	}

	return indices;
}

template <class... ComponentTypes>
ComponentRange<ComponentTypes...>::ComponentRange(ComponentMatrixOptConstT& componentMatrix)
	: m_componentVectors(FindComponentVectors(componentMatrix, DefaultIndices(componentMatrix), std::make_index_sequence<sizeof...(ComponentTypes)>())) {}


template <class... ComponentTypes>
ComponentRange<ComponentTypes...>::ComponentRange(ComponentMatrixOptConstT& componentMatrix, const std::vector<size_t>& componentVectorIndices)
	: m_componentVectors(FindComponentVectors(componentMatrix, componentVectorIndices, std::make_index_sequence<sizeof...(ComponentTypes)>())) {}


template <class... ComponentTypes>
auto ComponentRange<ComponentTypes...>::begin() -> iterator {
	return iterator{ &m_componentVectors, 0 };
}


template <class... ComponentTypes>
auto ComponentRange<ComponentTypes...>::end() -> iterator {
	return iterator{ &m_componentVectors, std::get<0>(m_componentVectors).size() };
}


template <class... ComponentTypes>
auto ComponentRange<ComponentTypes...>::begin() const -> const_iterator {
	return iterator{ &m_componentVectors, 0 };
}


template <class... ComponentTypes>
auto ComponentRange<ComponentTypes...>::end() const -> const_iterator {
	return iterator{ &m_componentVectors, std::get<0>(m_componentVectors).size() };
}


template <class... ComponentTypes>
auto ComponentRange<ComponentTypes...>::cbegin() const -> const_iterator {
	return iterator{ &m_componentVectors, 0 };
}


template <class... ComponentTypes>
auto ComponentRange<ComponentTypes...>::cend() const -> const_iterator {
	return iterator{ &m_componentVectors, std::get<0>(m_componentVectors).size() };
}


template <class... ComponentTypes>
template <size_t... Indices>
auto ComponentRange<ComponentTypes...>::FindComponentVectors(ComponentMatrixOptConstT& componentMatrix, const std::vector<size_t>& componentVectorIndices, std::index_sequence<Indices...>) -> ComponentVectorTupleT {
	return { componentMatrix.types[componentVectorIndices[Indices]].template get_vector<std::decay_t<ComponentTypes>>().Raw()... };
}


} // namespace inl::game