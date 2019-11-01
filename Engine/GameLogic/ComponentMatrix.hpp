#include "ComponentScheme.hpp"
#include "ComponentVector.hpp"

#include <BaseLibrary/Range.hpp>
#include <BaseLibrary/TemplateUtil.hpp>

namespace inl::game {


class ComponentMatrix;


template <bool Const>
class RefBase {
	template <bool ItConst>
	friend class IteratorBase;
	friend class ComponentMatrix;

public:
	RefBase(templ::add_const_conditional_t<ComponentMatrix, Const>& componentMatrix, size_t index)
		: m_componentMatrix(componentMatrix), m_index(index) {}

	template <class Component>
	templ::add_const_conditional_t<Component, Const>& GetComponent(size_t index) const;

	size_t GetComponentCount() const;

	RefBase& operator=(const RefBase& rhs);

private:
	templ::add_const_conditional_t<ComponentMatrix, Const>& m_componentMatrix;
	size_t m_index;
};



// Requirements:
// method of specifying permutation:
// - user wants to provide complete permutation (no work to do)
// - automatically match permutations (stable_sort both and match)
// method of selecting components:
// - subset: only a subset of source components used
// - superset: additional components specified as tuple
//
// Idea:
// Make an explicit class to represent user permutations.
// + Cache ordering of types in ComponentMatrices.


template <bool Const>
class RefBasePermutation {
public:
	RefBasePermutation(RefBase<Const> source);

	template <class Iter>
	RefBasePermutation(RefBase<Const> source, Iter firstIndex, Iter lastIndex);

	template <class Component>
	templ::add_const_conditional_t<Component, Const>& GetComponent(size_t index) const;

	size_t GetComponentCount() const;
};


template <bool Const>
class IteratorBase {
public:
	friend class ComponentMatrix;

	IteratorBase(templ::add_const_conditional_t<ComponentMatrix, Const>& componentMatrix, size_t index)
		: m_ref(componentMatrix, index) {}

	template <class Q = std::enable_if_t<!Const, IteratorBase<Const>>>
	operator Q() const {
		return Q{ m_ref.m_componentMatrix, m_ref.m_index };
	}

	RefBase<Const>& operator*() const { return m_ref; }
	RefBase<Const>* operator->() const { return &m_ref; }

	IteratorBase& operator++();
	IteratorBase& operator--();
	IteratorBase operator++(int);
	IteratorBase operator--(int);
	IteratorBase& operator+=(size_t n);
	IteratorBase& operator-=(size_t n);
	friend IteratorBase operator+(IteratorBase lhs, size_t n) { return lhs += n; }
	friend IteratorBase operator+(size_t n, IteratorBase rhs) { return rhs += n; }
	friend IteratorBase operator-(IteratorBase lhs, size_t n) { return lhs -= n; }
	friend size_t operator-(const IteratorBase& lhs, const IteratorBase& rhs) { return lhs.m_ref.index - rhs.m_ref.index; }

	friend auto operator<=>(const IteratorBase& lhs, const IteratorBase& rhs) { return lhs.m_ref.index <=> rhs.m_ref.index; }

private:
	RefBase<Const> m_ref;
};


class ComponentMatrix {
private:
	template <bool Const>
	class RefBase;

	using RefMut = RefBase<false>;
	using RefConst = RefBase<true>;

	using Iterator = IteratorBase<false>;
	using ConstIterator = IteratorBase<true>;

public:
	template <class... Components>
	void EmplaceBackEntity(Components&&... components);

	template <class... Components>
	Iterator EmplaceEntity(ConstIterator where, Components&&... components);

	Iterator InsertEntity(ConstIterator where);
	void PushBackEntity();
	void EraseEntity(ConstIterator where);
	void EraseEntity(ConstIterator first, ConstIterator last);

	Iterator BeginEntity();
	Iterator EndEntity();
	ConstIterator BeginEntity() const;
	ConstIterator EndEntity() const;
	ConstIterator CBeginEntity() const;
	ConstIterator CEndEntity() const;

	size_t SizeEntity() const;

private:
	std::vector<std::unique_ptr<ComponentVectorBase>> m_componentVectors;
};


template <bool Const>
template <class Component>
templ::add_const_conditional_t<Component, Const>& RefBase<Const>::GetComponent(size_t index) const {
	auto& container = *m_componentMatrix.m_componentVectors[index];
	return dynamic_cast<templ::add_const_conditional_t<Component, Const>&>(container)[m_index];
}


template <bool Const>
size_t RefBase<Const>::GetComponentCount() const {
	return m_componentMatrix.m_componentVectors.size();
}


template <bool Const>
RefBase<Const>& RefBase<Const>::operator=(const RefBase& rhs) {
	assert(false);
	return *this;
}


template <bool Const>
IteratorBase<Const>& IteratorBase<Const>::operator++() {
	++m_ref.index;
	return *this;
}


template <bool Const>
IteratorBase<Const>& IteratorBase<Const>::operator--() {
	--m_ref.index;
	return *this;
}


template <bool Const>
IteratorBase<Const> IteratorBase<Const>::operator++(int) {
	auto cpy = *this;
	++m_ref.index;
	return cpy;
}


template <bool Const>
IteratorBase<Const> IteratorBase<Const>::operator--(int) {
	auto cpy = *this;
	--m_ref.index;
	return cpy;
}


template <bool Const>
IteratorBase<Const>& IteratorBase<Const>::operator+=(size_t n) {
	m_ref.index += n;
	return *this;
}


template <bool Const>
IteratorBase<Const>& IteratorBase<Const>::operator-=(size_t n) {
	m_ref.index -= n;
	return *this;
}


template <class... Components>
void ComponentMatrix::EmplaceBackEntity(Components&&... components) {
	EmplaceEntity(EndEntity(), std::forward<Components>(components)...);
}


template <size_t N>
auto StableSortTypes(std::array<std::type_index, N> types) {
	struct Aggregate {
		std::type_index type;
		size_t index;
		bool operator<(const Aggregate& rhs) const { return type < rhs.type; }
	};
	std::array<Aggregate, N> sortedAggregates;
	for (auto i : Range(N)) {
		sortedAggregates[i].type = types[i];
		sortedAggregates[i].index = i;
	}
	std::stable_sort(sortedAggregates);
	std::array<size_t, N> sortedIndices;
	for (auto i : Range(N)) {
		sortedIndices[i] = sortedAggregates[i].index;
	}
	return sortedIndices;
}


template <class T, size_t N>
std::array<T, N> InvertPermutation(const std::array<T, N>& perm) {
	std::array<T, N> iperm;
	for (size_t i = 0; i < N; ++i) {
		iperm[perm[i]] = i;
	}
	return iperm;
}


template <class T, size_t N>
std::array<T, N> CombinePermutations(const std::array<T, N>& perm1, const std::array<T, N>& perm2) {
	std::array<T, N> cperm;
	for (size_t i = 0; i < N; ++i) {
		cperm = perm2[perm1[i]];
	}
	return cperm;
}


template <int I, class... Components>
void EmplaceComponent(std::vector<std::unique_ptr<ComponentVectorBase>>& containers, std::array<size_t, sizeof...(Components)> permutation, size_t where, Components&&... components) {
	if (I < sizeof...(Components)) {
		auto&& component = std::get<I>(std::forward_as_tuple(std::forward<Components>(components)...));
		containers[permutation[I]]->Emplace(where, std::forward<decltype(component)>(component)...);
		EmplaceComponent<I + 1>(std::forward<Components>(components)...);
	}
}


template <class... Components>
ComponentMatrix::Iterator ComponentMatrix::EmplaceEntity(ConstIterator where, Components&&... components) {
	if (sizeof...(Components) != m_componentVectors.size()) {
		throw InvalidArgumentException("Component sets are different.");
	}

	auto argsOrder = InvertPermutation(StableSortTypes({ typeid(Components)... }));

	std::array<std::type_index, sizeof...(Components)> containerTypes;
	for (auto i : Range(sizeof...(Components))) {
		containerTypes[i] = m_componentVectors[i]->Type();
	}
	auto containersOrder = StableSortTypes(containerTypes);

	auto perm = CombinePermutations(argsOrder, containersOrder);

	EmplaceComponent<0>(m_componentVectors, perm, where.m_ref.m_index, std::forward<Components>(components)...);

	return Iterator{ *this, where.m_ref.m_index };
}


} // namespace inl::game