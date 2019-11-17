#pragma once

#include "ComponentScheme.hpp"
#include "ComponentVector.hpp"

#include <BaseLibrary/Range.hpp>
#include <BaseLibrary/TemplateUtil.hpp>

#include <array>
#include <optional>

namespace inl::game {

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

class ComponentMatrix;
struct EntityTypeMatrix : public std::vector<std::unique_ptr<ComponentVectorBase>> {
	EntityTypeMatrix(ComponentMatrix& parent) : m_parent(parent) {}
	ComponentMatrix& m_parent;
};


//------------------------------------------------------------------------------
// EntityVector
//------------------------------------------------------------------------------

class EntityVector {
public:
	template <bool Const>
	class generic_reference {
		friend generic_reference<!Const>;

	public:
		generic_reference() : m_index(std::numeric_limits<decltype(m_index)>::max()), m_matrix(nullptr) {}
		generic_reference(size_t index, templ::add_const_conditional_t<EntityTypeMatrix, Const>& matrix) : m_index(index), m_matrix(&matrix) {}

		generic_reference(const generic_reference&) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_reference(const generic_reference<false>& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		template <class AsType>
		templ::add_const_conditional_t<AsType, Const>& get(size_t index) const;
		size_t size() const;
		std::type_index get_type(size_t index) const;
		generic_reference& operator=(const generic_reference<true>& rhs);
		generic_reference& operator=(generic_reference<false>&& rhs);

		template <class... Components>
		generic_reference& assign_extend(const generic_reference<true>&, Components&&... rhs);

		template <class... Components>
		generic_reference& assign_extend(generic_reference<false>&&, Components&&... rhs);

		template <class SkipPred>
		generic_reference& assign_partial(const generic_reference<true>&, SkipPred skipPred);

		template <class SkipPred>
		generic_reference& assign_partial(generic_reference<false>&&, SkipPred skipPred);

		generic_reference* operator->() { return this; }

	private:
		template <class other_t>
		const std::vector<bool>& assign_auto_mask(other_t&& rhs);

		template <class Pred, class... Components>
		void assign_extra_mask(Pred pred, Components&&... components);

		template <class other_t, class SkipPred>
		void assign_partial(other_t&&, SkipPred skipPred, int);

		void assign_same(const generic_reference<true>& rhs);
		void assign_same(generic_reference<false>&& rhs);
		void assign_auto(const generic_reference<true>& rhs);
		void assign_auto(generic_reference<false>&& rhs);

	private:
		size_t m_index;
		templ::add_const_conditional_t<EntityTypeMatrix, Const>* m_matrix;
	};

	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;

	public:
		generic_iterator() : m_index(std::numeric_limits<decltype(m_index)>::max()), m_matrix(nullptr) {}
		generic_iterator(size_t index, templ::add_const_conditional_t<EntityTypeMatrix, Const>& matrix) : m_index(index), m_matrix(&matrix) {}

		generic_iterator(const generic_iterator&) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		generic_reference<Const> operator*() const { return generic_reference<Const>{ m_index, *m_matrix }; }
		generic_reference<Const> operator->() const { return generic_reference<Const>{ m_index, *m_matrix }; }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		generic_iterator& operator+=(size_t n);
		generic_iterator& operator-=(size_t n);
		friend generic_iterator operator+(generic_iterator lhs, size_t n) { return lhs += n; }
		friend generic_iterator operator+(size_t n, generic_iterator rhs) { return rhs += n; }
		friend generic_iterator operator-(generic_iterator lhs, size_t n) { return lhs -= n; }
		friend size_t operator-(const generic_iterator& lhs, const generic_iterator& rhs) { return lhs.m_index - rhs.m_index; }
		auto operator<=>(const generic_iterator& rhs) const { return get_index() <=> rhs.get_index(); }
		auto operator==(const generic_iterator& rhs) const { return get_index() == rhs.get_index(); }

		size_t get_index() const { return m_index; }

	private:
		size_t m_index;
		templ::add_const_conditional_t<EntityTypeMatrix, Const>* m_matrix;
	};

public:
	using size_type = size_t;
	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;
	using reference = generic_reference<false>;
	using const_reference = generic_reference<true>;

public:
	EntityVector(EntityTypeMatrix& matrix) : m_matrix(matrix) {}

	// Element access
	reference operator[](size_t index);
	const_reference operator[](size_t index) const;
	reference back();
	reference front();
	const_reference back() const;
	const_reference front() const;

	// Iterators
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	// Capacity
	size_type size() const;
	bool empty() const;

	// Modifiers
	void insert(const_iterator where, const_reference entity);
	template <class... Components>
	void emplace(const_iterator where, Components&&... components);
	void erase(const_iterator where);
	void push_back(const_reference entity) { insert(end(), entity); }
	template <class... Components>
	void emplace_back(Components&&... components) { emplace(end(), std::forward<Components>(components)...); }

private:
	template <int Index, class... Components>
	void emplace_recurse(const_iterator where, const std::array<size_t, sizeof...(Components)>& pairing, const std::tuple<Components&&...>& args);

private:
	EntityTypeMatrix& m_matrix;
};


//------------------------------------------------------------------------------
// TypeVector
//------------------------------------------------------------------------------

class TypeVector {
public:
	template <bool Const>
	class generic_reference {
		friend generic_reference<!Const>;

	public:
		generic_reference() : m_index(std::numeric_limits<decltype(m_index)>::max()), m_matrix(nullptr) {}
		generic_reference(size_t index, templ::add_const_conditional_t<EntityTypeMatrix, Const>& matrix) : m_index(index), m_matrix(&matrix) {}

		generic_reference(const generic_reference&) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_reference(const generic_reference<false>& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		template <class const_reference_t = std::enable_if_t<!Const, generic_reference<Const>>>
		operator const_reference_t() const { return const_reference_t{ m_index }; }

		template <class AsType>
		templ::add_const_conditional_t<AsType, Const>& get(size_t index) const;
		template <class AsType>
		templ::add_const_conditional_t<_ComponentVector<AsType>, Const>& get_vector() const;
		size_t size() const;
		std::type_index get_type() const;

		generic_reference* operator->() { return this; }

	private:
		size_t m_index;
		templ::add_const_conditional_t<EntityTypeMatrix, Const>* m_matrix;
	};


	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;

	public:
		generic_iterator() : m_index(std::numeric_limits<decltype(m_index)>::max()), m_matrix(nullptr) {}
		generic_iterator(size_t index, templ::add_const_conditional_t<EntityTypeMatrix, Const>& matrix) : m_index(index), m_matrix(&matrix) {}

		generic_iterator(const generic_iterator&) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		generic_reference<Const> operator*() const { return generic_reference<Const>{ m_index, *m_matrix }; }
		generic_reference<Const> operator->() const { return generic_reference<Const>{ m_index, *m_matrix }; }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		generic_iterator& operator+=(size_t n);
		generic_iterator& operator-=(size_t n);
		friend generic_iterator operator+(generic_iterator lhs, size_t n) { return lhs += n; }
		friend generic_iterator operator+(size_t n, generic_iterator rhs) { return rhs += n; }
		friend generic_iterator operator-(generic_iterator lhs, size_t n) { return lhs -= n; }
		friend size_t operator-(const generic_iterator& lhs, const generic_iterator& rhs) { return lhs.m_index - rhs.m_index; }
		auto operator<=>(const generic_iterator& rhs) const { return get_index() <=> rhs.get_index(); }
		auto operator==(const generic_iterator& rhs) const { return get_index() == rhs.get_index(); }

		size_t get_index() const { return m_index; }

	private:
		size_t m_index;
		templ::add_const_conditional_t<EntityTypeMatrix, Const>* m_matrix;
	};

public:
	using size_type = std::size_t;

	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;
	using reference = generic_reference<false>;
	using const_reference = generic_reference<true>;

public:
	TypeVector(EntityTypeMatrix& matrix) : m_matrix(matrix) {}

	TypeVector& operator=(const TypeVector& rhs);

	// Element access
	reference operator[](size_t index);
	const_reference operator[](size_t index) const;

	// Iterators
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	// Capacity
	size_type size() const;

	// Modifiers
	template <class T>
	void insert(const_iterator where, const _ComponentVector<T>& components);
	template <class T>
	void insert(const_iterator where, _ComponentVector<T>&& components);

	template <class T, class... Args>
	void emplace(const_iterator where, Args&&... args);

	void erase(const_iterator where);

	template <class T>
	void push_back(const _ComponentVector<T>& components) { insert(end(), components); }
	template <class T>
	void push_back(_ComponentVector<T>&& components) { insert(end(), std::move(components)); }

	template <class T, class... Args>
	void emplace_back(Args&&... args) { return emplace(end(), std::forward<Args>(args)...); }

	// Additional
	auto& type_order() const { return m_orderedTypes; }
	auto equal_range(std::type_index type) const -> std::pair<std::vector<std::pair<std::type_index, size_t>>::const_iterator, std::vector<std::pair<std::type_index, size_t>>::const_iterator>;

private:
	void recompute_order();

private:
	std::vector<std::pair<std::type_index, size_t>> m_orderedTypes;
	EntityTypeMatrix& m_matrix;
};


//------------------------------------------------------------------------------
// ComponentMatrix
//------------------------------------------------------------------------------


class ComponentMatrix {
public:
	ComponentMatrix() : m_matrix(*this), entities(m_matrix), types(m_matrix) {}

	EntityVector entities;
	TypeVector types;

private:
	EntityTypeMatrix m_matrix;
};


//------------------------------------------------------------------------------
// Helpers -- implementation
//------------------------------------------------------------------------------

inline auto noOpLambda = [](auto) {};
inline auto noOpLambdaFalse = [](auto) { return false; };

template <class TarIter,
		  class SrcIter,
		  class OnMatch,
		  class OnMismatch = decltype(noOpLambda),
		  class SkipPredTar = decltype(noOpLambdaFalse),
		  class SkipPredSrc = decltype(noOpLambdaFalse)>
void PairComponents(
	TarIter tarFirst,
	TarIter tarLast,
	SrcIter srcFirst,
	SrcIter srcLast,
	OnMatch onMatch,
	OnMismatch onMismatch = noOpLambda,
	SkipPredTar skipPredTar = noOpLambdaFalse,
	SkipPredSrc skipPredSrc = noOpLambdaFalse) {
	while (tarFirst != tarLast && srcFirst != srcLast) {
		if (skipPredTar(*tarFirst)) {
			++tarFirst;
		}
		else if (skipPredSrc(*srcFirst)) {
			++srcFirst;
		}
		else if (tarFirst->first < srcFirst->first) {
			onMismatch(*tarFirst);
			++tarFirst;
		}
		else if (tarFirst->first > srcFirst->first) {
			++srcFirst;
		}
		else {
			onMatch(*tarFirst, *srcFirst);
			++tarFirst;
			++srcFirst;
		}
	}
}


template <class... Components>
struct SortedComponents {
	static constexpr size_t count = sizeof...(Components);
	static const std::array<std::pair<std::type_index, size_t>, count> order;
};

template <class... Components>
const std::array<std::pair<std::type_index, size_t>, SortedComponents<Components...>::count> SortedComponents<Components...>::order = [] {
	std::array<std::pair<std::type_index, size_t>, SortedComponents<Components...>::count> arr = { std::pair<std::type_index, size_t>{ typeid(Components), 0 }... };
	for (auto i : Range(SortedComponents<Components...>::count))
		arr[i].second = i;
	std::stable_sort(arr.begin(), arr.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});
	return arr;
}();


//------------------------------------------------------------------------------
// EntityVector::generic_reference -- implementation
//------------------------------------------------------------------------------

template <bool Const>
template <class AsType>
templ::add_const_conditional_t<AsType, Const>& EntityVector::generic_reference<Const>::get(size_t index) const {
	auto& vec = dynamic_cast<templ::add_const_conditional_t<_ComponentVector<AsType>&, Const>>(*(*m_matrix)[index]);
	return vec[m_index];
}

template <bool Const>
size_t EntityVector::generic_reference<Const>::size() const {
	assert(m_matrix);
	return m_matrix->size();
}

template <bool Const>
std::type_index EntityVector::generic_reference<Const>::get_type(size_t index) const {
	return (*m_matrix)[index]->Type();
}

template <bool Const>
EntityVector::generic_reference<Const>& EntityVector::generic_reference<Const>::operator=(const generic_reference<true>& rhs) {
	if (m_matrix && m_matrix == rhs.m_matrix) {
		if (m_index != rhs.m_index) {
			assign_same(rhs);
		}
	}
	else if (m_matrix && rhs.m_matrix) {
		assign_auto(rhs);
	}
	return *this;
}

template <bool Const>
EntityVector::generic_reference<Const>& EntityVector::generic_reference<Const>::operator=(generic_reference<false>&& rhs) {
	if (m_matrix && m_matrix == rhs.m_matrix) {
		if (m_index != rhs.m_index) {
			assign_same(std::move(rhs));
		}
	}
	else if (m_matrix && rhs.m_matrix) {
		assign_auto(std::move(rhs));
	}
	return *this;
}

template <bool Const>
template <class... Components>
EntityVector::generic_reference<Const>& EntityVector::generic_reference<Const>::assign_extend(const generic_reference<true>& rhs, Components&&... components) {
	if (m_matrix && rhs.m_matrix) {
		const std::vector<bool>& mask = assign_auto_mask(rhs);
		assign_extra_mask([&](const auto& tar) { return mask[tar.second]; }, std::forward<Components>(components)...);
	}
	else if (m_matrix) {
		assign_extra_mask([&](const auto& tar) { return false; }, std::forward<Components>(components)...);
	}

	return *this;
}

template <bool Const>
template <class... Components>
EntityVector::generic_reference<Const>& EntityVector::generic_reference<Const>::assign_extend(generic_reference<false>&& rhs, Components&&... components) {
	if (m_matrix && rhs.m_matrix) {
		const std::vector<bool>& mask = assign_auto_mask(rhs);
		assign_extra_mask([&](const auto& tar) { return mask[tar.second]; }, std::forward<Components>(components)...);
	}
	else if (m_matrix) {
		assign_extra_mask([&](const auto& tar) { return false; }, std::forward<Components>(components)...);
	}

	return *this;
}

template <bool Const>
template <class SkipPred>
EntityVector::generic_reference<Const>& EntityVector::generic_reference<Const>::assign_partial(const generic_reference<true>& rhs, SkipPred skipPred) {
	if (m_matrix && rhs.m_matrix) {
		assign_partial(rhs, skipPred, int());
	}
	return *this;
}

template <bool Const>
template <class SkipPred>
EntityVector::generic_reference<Const>& EntityVector::generic_reference<Const>::assign_partial(generic_reference<false>&& rhs, SkipPred skipPred) {
	if (m_matrix && rhs.m_matrix) {
		assign_partial(rhs, skipPred, int());
	}
	return *this;
}


template <bool Const>
template <class other_t>
const std::vector<bool>& EntityVector::generic_reference<Const>::assign_auto_mask(other_t&& rhs) {
	thread_local std::vector<bool> mask;
	mask.resize(size(), false);

	auto& lhsOrder = m_matrix->m_parent.types.type_order();
	auto& rhsOrder = rhs.m_matrix->m_parent.types.type_order();

	PairComponents(lhsOrder.begin(), lhsOrder.end(), rhsOrder.begin(), rhsOrder.end(), [&](const auto& tar, const auto& src) {
		auto& lhsVec = (*m_matrix)[tar.second];
		auto& rhsVec = (*rhs.m_matrix)[src.second];
		mask[tar.second] = true;
		if constexpr (std::is_rvalue_reference_v<decltype(rhs)>) {
			lhsVec->Move(m_index, *rhsVec, rhs.m_index);
		}
		else {
			lhsVec->Copy(m_index, *rhsVec, rhs.m_index);
		}
	});

	return mask;
}

template <bool Const>
template <class Pred, class... Components>
void EntityVector::generic_reference<Const>::assign_extra_mask(Pred pred, Components&&... components) {
	std::array<size_t, sizeof...(Components)> pairing;
	memset(&pairing, 0xFF, sizeof(pairing));

	auto onMatch = [&](const auto& tar, const auto& src) {
		size_t thisIndex = tar.second;
		size_t argIndex = src.second;
		pairing[argIndex] = thisIndex;
	};

	auto& lhsOrder = m_matrix->m_parent.types.type_order();
	const auto& argOrder = SortedComponents<Components...>::order;
	PairComponents(lhsOrder.begin(), lhsOrder.end(), argOrder.begin(), argOrder.end(), onMatch, noOpLambda, pred);

	// TODO: assign recurse
}


template <bool Const>
template <class other_t, class SkipPred>
void EntityVector::generic_reference<Const>::assign_partial(other_t&& rhs, SkipPred skipPred, int) {
	auto& lhsOrder = m_matrix->m_parent.types.type_order();
	auto& rhsOrder = rhs.m_matrix->m_parent.types.type_order();

	auto onMatch = [&](const auto& tar, const auto& src) {
		auto& lhsVec = (*m_matrix)[tar.second];
		auto& rhsVec = (*rhs.m_matrix)[src.second];
		if constexpr (std::is_rvalue_reference_v<decltype(rhs)>) {
			lhsVec->Move(m_index, *rhsVec, rhs.m_index);
		}
		else {
			lhsVec->Copy(m_index, *rhsVec, rhs.m_index);
		}
	};
	auto skipSource = [&](const auto& record) {
		const auto& type = record.first;
		const auto& index = record.second; // real index, not type-ordered index
		return skipPred(type, index);
	};

	PairComponents(lhsOrder.begin(), lhsOrder.end(),
				   rhsOrder.begin(), rhsOrder.end(),
				   onMatch, noOpLambda,
				   noOpLambdaFalse, skipSource);
}


template <bool Const>
void EntityVector::generic_reference<Const>::assign_same(const generic_reference<true>& rhs) {
	assert(m_matrix == rhs.m_matrix);
	for (auto i : Range(size())) {
		auto& lhsVec = (*m_matrix)[i];
		auto& rhsVec = (*rhs.m_matrix)[i];
		lhsVec->Copy(m_index, *rhsVec, rhs.m_index);
	}
}

template <bool Const>
void EntityVector::generic_reference<Const>::assign_same(generic_reference<false>&& rhs) {
	assert(m_matrix == rhs.m_matrix);
	for (auto i : Range(size())) {
		auto& lhsVec = (*m_matrix)[i];
		auto& rhsVec = (*rhs.m_matrix)[i];
		lhsVec->Move(m_index, *rhsVec, rhs.m_index);
	}
}


template <bool Const>
void EntityVector::generic_reference<Const>::assign_auto(const generic_reference<true>& rhs) {
	auto& lhsOrder = m_matrix->m_parent.types.type_order();
	auto& rhsOrder = rhs.m_matrix->m_parent.types.type_order();

	PairComponents(lhsOrder.begin(), lhsOrder.end(), rhsOrder.begin(), rhsOrder.end(), [&](const auto& tar, const auto& src) {
		auto& lhsVec = (*m_matrix)[tar.second];
		auto& rhsVec = (*rhs.m_matrix)[src.second];
		lhsVec->Copy(m_index, *rhsVec, rhs.m_index);
	});
}

template <bool Const>
void EntityVector::generic_reference<Const>::assign_auto(generic_reference<false>&& rhs) {
	auto& lhsOrder = m_matrix->m_parent.types.type_order();
	auto& rhsOrder = rhs.m_matrix->m_parent.types.type_order();

	PairComponents(lhsOrder.begin(), lhsOrder.end(), rhsOrder.begin(), rhsOrder.end(), [&](const auto& tar, const auto& src) {
		auto& lhsVec = (*m_matrix)[tar.second];
		auto& rhsVec = (*rhs.m_matrix)[src.second];
		lhsVec->Move(m_index, *rhsVec, rhs.m_index);
	});
}


//------------------------------------------------------------------------------
// EntityVector::generic_iterator -- implementation
//------------------------------------------------------------------------------

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator++() {
	++m_index;
	return *this;
}

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator--() {
	--m_index;
	return *this;
}

template <bool Const>
EntityVector::generic_iterator<Const> EntityVector::generic_iterator<Const>::operator++(int) {
	auto copy = *this;
	operator++();
	return copy;
}

template <bool Const>
EntityVector::generic_iterator<Const> EntityVector::generic_iterator<Const>::operator--(int) {
	auto copy = *this;
	operator--();
	return copy;
}

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator+=(size_t n) {
	m_index += n;
	return *this;
}

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator-=(size_t n) {
	m_index -= n;
	return *this;
}


//------------------------------------------------------------------------------
// EntityVector -- implementation
//------------------------------------------------------------------------------


template <class... Components>
void EntityVector::emplace(const_iterator where, Components&&... components) {
	auto index = where.get_index();

	const auto& thisOrder = m_matrix.m_parent.types.type_order();
	const auto& argOrder = SortedComponents<Components...>::order;

	std::array<size_t, sizeof...(Components)> pairing;
	memset(&pairing, 0xFF, sizeof(pairing));

	auto onMatch = [&](const auto& tar, const auto& src) {
		size_t thisIndex = tar.second;
		size_t argIndex = src.second;
		pairing[argIndex] = thisIndex;
	};
	auto onMismatch = [&](const auto& tar) {
		m_matrix[tar.second]->InsertDefault(where.get_index());
	};
	PairComponents(thisOrder.begin(), thisOrder.end(), argOrder.begin(), argOrder.end(), onMatch, onMismatch);

	emplace_recurse<0>(where, pairing, std::forward_as_tuple(std::forward<Components>(components)...));
}

template <int Index, class... Components>
void EntityVector::emplace_recurse(const_iterator where, const std::array<size_t, sizeof...(Components)>& pairing, const std::tuple<Components&&...>& args) {
	if constexpr (Index < sizeof...(Components)) {
		size_t thisIndex = pairing[Index];
		if (thisIndex < std::numeric_limits<size_t>::max()) {
			auto& thisVector = dynamic_cast<_ComponentVector<std::decay_t<std::tuple_element_t<Index, std::tuple<Components&&...>>>>&>(*m_matrix[thisIndex]);
			thisVector.Insert(where.get_index(), std::get<Index>(args));
		}
		emplace_recurse<Index + 1>(where, pairing, args);
	}
}


//------------------------------------------------------------------------------
// TypeVector::generic_reference -- implementation
//------------------------------------------------------------------------------


template <bool Const>
template <class AsType>
templ::add_const_conditional_t<AsType, Const>& TypeVector::generic_reference<Const>::get(size_t index) const {
	auto& vec = dynamic_cast<templ::add_const_conditional_t<_ComponentVector<AsType>&, Const>>(*(*m_matrix)[m_index]);
	return vec[index];
}

template <bool Const>
template <class AsType>
templ::add_const_conditional_t<_ComponentVector<AsType>, Const>& TypeVector::generic_reference<Const>::get_vector() const {
	auto& vec = dynamic_cast<templ::add_const_conditional_t<_ComponentVector<AsType>&, Const>>(*(*m_matrix)[m_index]);
	return vec;
}

template <bool Const>
size_t TypeVector::generic_reference<Const>::size() const {
	assert(m_matrix);
	return (*m_matrix)[m_index]->Size();
}

template <bool Const>
std::type_index TypeVector::generic_reference<Const>::get_type() const {
	return (*m_matrix)[m_index]->Type();
}


//------------------------------------------------------------------------------
// TypeVector::generic_iterator -- implementation
//------------------------------------------------------------------------------

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator++() {
	++m_index;
	return *this;
}

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator--() {
	--m_index;
	return *this;
}

template <bool Const>
TypeVector::generic_iterator<Const> TypeVector::generic_iterator<Const>::operator++(int) {
	auto copy = *this;
	operator++();
	return copy;
}

template <bool Const>
TypeVector::generic_iterator<Const> TypeVector::generic_iterator<Const>::operator--(int) {
	auto copy = *this;
	operator--();
	return copy;
}

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator+=(size_t n) {

	m_index += n;
	return *this;
}

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator-=(size_t n) {
	m_index -= n;
	return *this;
}


//------------------------------------------------------------------------------
// TypeVector -- implementation
//------------------------------------------------------------------------------

template <class T>
void TypeVector::insert(const_iterator where, const _ComponentVector<T>& components) {
	auto newContainer = std::make_unique<_ComponentVector<T>>(components);
	newContainer->Resize(m_matrix.empty() ? 0 : m_matrix[0]->Size());
	m_matrix.push_back(std::move(newContainer));
	recompute_order();
}

template <class T>
void TypeVector::insert(const_iterator where, _ComponentVector<T>&& components) {
	auto newContainer = std::make_unique<_ComponentVector<T>>(std::move(components));
	newContainer->Resize(m_matrix.empty() ? 0 : m_matrix[0]->Size());
	m_matrix.push_back(std::move(newContainer));
	recompute_order();
}

template <class T, class... Args>
void TypeVector::emplace(const_iterator where, Args&&... args) {
	m_matrix.push_back(std::make_unique<_ComponentVector<T>>(std::forward<Args>(args)...));
	recompute_order();
}


} // namespace inl::game