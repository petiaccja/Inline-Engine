#pragma once

#include "ComponentStore.hpp"

#include <map>
#include <vector>
#include <type_traits>


namespace inl::game {


namespace impl {
	template <class T, bool Const>
	using add_const_opt_t = std::conditional_t<Const, const T, std::remove_const_t<T>>;

	template <class T>
	using ComponentVectorT = ContiguousVector<std::decay_t<T>>;

	template <class T>
	using ComponentVectorOptConstT = add_const_opt_t<ComponentVectorT<T>, std::is_const_v<std::remove_reference_t<T>>>;
} // namespace impl



template <class... ComponentTypes>
class ComponentRange {
	static constexpr bool IsAllConst = std::conjunction_v<std::is_const<std::remove_reference_t<ComponentTypes>>...>;
	using ComponentStoreOptConstT = impl::add_const_opt_t<ComponentStore, IsAllConst>;
	using ComponentVectorTupleT = std::tuple<impl::ComponentVectorOptConstT<ComponentTypes>&...>;

public:
	ComponentRange(ComponentStoreOptConstT& componentStore);
	ComponentRange(ComponentStoreOptConstT& componentStore, const std::vector<size_t>& componentVectorIndices);

private:
	std::vector<size_t> DefaultIndices(ComponentStoreOptConstT& componentStore);

	template <size_t... Indices>
	ComponentVectorTupleT FindComponentVectors(ComponentStoreOptConstT& componentStore, const std::vector<size_t>& componentVectorIndices, std::index_sequence<Indices...>);

private:
	ComponentVectorTupleT m_componentVectors;
};


template <class... ComponentTypes>
std::vector<size_t> ComponentRange<ComponentTypes...>::DefaultIndices(ComponentStoreOptConstT& componentStore) {
	std::map<std::type_index, size_t> indexCounter;
	std::array types = { std::type_index(typeid(ComponentTypes))... };
	const ComponentScheme& scheme = componentStore.Scheme();
	std::vector<size_t> indices;

	for (auto& type : types) {
		auto it = indexCounter.find(type);
		if (it == indexCounter.end()) {
			auto [firstIdx, lastIdx] = scheme.Index(type);
			indexCounter[type] = firstIdx;
			indices.push_back(firstIdx);
		}
		else {
			indices.push_back(++it->second);
		}
	}

	return indices;
}

template <class... ComponentTypes>
ComponentRange<ComponentTypes...>::ComponentRange(ComponentStoreOptConstT& componentStore)
	: m_componentVectors(FindComponentVectors(componentStore, DefaultIndices(componentStore), std::make_index_sequence<sizeof...(ComponentTypes)>())) {}


template <class... ComponentTypes>
ComponentRange<ComponentTypes...>::ComponentRange(ComponentStoreOptConstT& componentStore, const std::vector<size_t>& componentVectorIndices)
	: m_componentVectors(FindComponentVectors(componentStore, componentVectorIndices, std::make_index_sequence<sizeof...(ComponentTypes)>())) {}


template <class... ComponentTypes>
template <size_t... Indices>
auto ComponentRange<ComponentTypes...>::FindComponentVectors(ComponentStoreOptConstT& componentStore, const std::vector<size_t>& componentVectorIndices, std::index_sequence<Indices...>) -> ComponentVectorTupleT {
	return { componentStore.template GetComponentVector<std::decay_t<ComponentTypes>>(componentVectorIndices[Indices])... };
}


} // namespace inl::game