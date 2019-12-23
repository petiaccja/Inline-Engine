#pragma once

#include "ComponentVector.hpp"

#include <BaseLibrary/Range.hpp>
#include <BaseLibrary/TemplateUtil.hpp>

#include <array>
#include <typeindex>
#include <utility>


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
	class generic_iterator;

	class value_type {
		template <bool Const>
		friend class generic_iterator;

	public:
		value_type() : m_index(std::numeric_limits<decltype(m_index)>::max()), m_matrix(nullptr) {}
		value_type(size_t index, EntityTypeMatrix* matrix) : m_index(index), m_matrix(matrix) {}

		value_type(const value_type&) = delete;
		value_type(value_type&& rhs) noexcept : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		value_type& operator=(const value_type& rhs);
		value_type& operator=(value_type&& rhs);

		template <class... Components>
		value_type& assign_extend(const value_type&, Components&&... rhs);

		template <class... Components>
		value_type& assign_extend(value_type&&, Components&&... rhs);

		template <class SkipPred>
		value_type& assign_partial(const value_type&, SkipPred skipPred);

		template <class SkipPred>
		value_type& assign_partial(value_type&&, SkipPred skipPred);

		template <class AsType>
		decltype(auto) get(size_t index);

		template <class AsType>
		decltype(auto) get(size_t index) const;

		size_t size() const;
		std::type_index get_type(size_t index) const;

	private:
		template <class other_t>
		const std::vector<bool>& assign_auto_mask(other_t&& rhs);

		template <class Pred, class... Components>
		void assign_extra_mask(Pred pred, Components&&... components);

		template <class other_t, class SkipPred>
		void assign_partial(other_t&&, SkipPred skipPred, int);

		void assign_same(const value_type& rhs);
		void assign_same(value_type&& rhs);
		void assign_auto(const value_type& rhs);
		void assign_auto(value_type&& rhs);
		template <int Index, class... Components>
		void assign_recurse(const std::array<size_t, sizeof...(Components)>& pairing, const std::tuple<Components&&...>& args);

	private:
		size_t m_index;
		EntityTypeMatrix* m_matrix;
	};

	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;

	public:
		generic_iterator() : m_value(0, nullptr) {}
		generic_iterator(size_t index, EntityTypeMatrix* matrix) : m_value(index, matrix) {}

		generic_iterator(const generic_iterator& rhs) : m_value(rhs.m_value.m_index, rhs.m_value.m_matrix) {}
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_value(rhs.m_value.m_index, rhs.m_value.m_matrix) {}

		templ::add_const_conditional_t<value_type, Const>& operator*() const { return m_value; }
		templ::add_const_conditional_t<value_type, Const>* operator->() const { return &m_value; }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		generic_iterator& operator+=(size_t n);
		generic_iterator& operator-=(size_t n);
		friend generic_iterator operator+(generic_iterator lhs, size_t n) { return generic_iterator{ lhs } += n; }
		friend generic_iterator operator+(size_t n, generic_iterator rhs) { return generic_iterator{ rhs } += n; }
		friend generic_iterator operator-(generic_iterator lhs, size_t n) { return generic_iterator{ lhs } -= n; }
		friend size_t operator-(const generic_iterator& lhs, const generic_iterator& rhs) { return lhs.get_index() - rhs.get_index(); }
		auto operator<=>(const generic_iterator& rhs) const { return get_index() <=> rhs.get_index(); }
		auto operator==(const generic_iterator& rhs) const { return get_index() == rhs.get_index(); }
		auto operator!=(const generic_iterator& rhs) const { return get_index() != rhs.get_index(); }

		size_t get_index() const { return m_value.m_index; }

	private:
		mutable value_type m_value;
	};

public:
	using size_type = size_t;
	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using rvalue_reference = value_type&&;

public:
	EntityVector(EntityTypeMatrix& matrix) : m_matrix(matrix) {}

	// Element access
	value_type operator[](size_t index);
	const value_type operator[](size_t index) const;
	value_type back();
	value_type front();
	const value_type back() const;
	const value_type front() const;

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
	void clear();
	void resize(size_t size);
	void reserve(size_t capacity);

	// Modifiers
	void insert(const_iterator where, const_reference entity);
	void insert(const_iterator where, reference&& entity);
	template <class... Components>
	void emplace(const_iterator where, Components&&... components);
	void erase(const_iterator where);
	void push_back(const_reference entity) { insert(end(), entity); }
	void push_back(reference&& entity) { insert(end(), std::move(entity)); }
	template <class... Components>
	void emplace_back(Components&&... components) { emplace(end(), std::forward<Components>(components)...); }
	void pop_back();

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
	class generic_iterator;

	class value_type {
		template <bool Const>
		friend class generic_iterator;

	public:
		value_type() : m_index(0), m_matrix(nullptr) {}
		value_type(size_t index, EntityTypeMatrix* matrix) : m_index(index), m_matrix(matrix) {}

		value_type(const value_type&) = delete;
		value_type(value_type&& rhs) noexcept : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		template <class AsType>
		AsType& get(size_t index);
		template <class AsType>
		const AsType& get(size_t index) const;

		template <class AsType>
		ComponentVector<AsType>& get_vector();
		template <class AsType>
		const ComponentVector<AsType>& get_vector() const;

		size_t size() const;
		std::type_index get_type() const;

	private:
		size_t m_index;
		EntityTypeMatrix* m_matrix;
	};


	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;

	public:
		generic_iterator() : m_value(0, nullptr) {}
		generic_iterator(size_t index, EntityTypeMatrix* matrix) : m_value(index, matrix) {}

		generic_iterator(const generic_iterator& rhs) : m_value(rhs.m_value.m_index, rhs.m_value.m_matrix) {}
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_value(rhs.m_value.m_index, rhs.m_value.m_matrix) {}

		templ::add_const_conditional_t<value_type, Const>& operator*() const { return m_value; }
		templ::add_const_conditional_t<value_type, Const>* operator->() const { return &m_value; }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		generic_iterator& operator+=(size_t n);
		generic_iterator& operator-=(size_t n);
		friend generic_iterator operator+(generic_iterator lhs, size_t n) { return generic_iterator{ lhs } += n; }
		friend generic_iterator operator+(size_t n, generic_iterator rhs) { return generic_iterator{ rhs } += n; }
		friend generic_iterator operator-(generic_iterator lhs, size_t n) { return generic_iterator{ lhs } -= n; }
		friend size_t operator-(const generic_iterator& lhs, const generic_iterator& rhs) { return lhs.get_index() - rhs.get_index(); }
		auto operator<=>(const generic_iterator& rhs) const { return get_index() <=> rhs.get_index(); }
		auto operator==(const generic_iterator& rhs) const { return get_index() == rhs.get_index(); }
		auto operator!=(const generic_iterator& rhs) const { return get_index() != rhs.get_index(); }

		size_t get_index() const { return m_value.m_index; }

	private:
		mutable value_type m_value;
	};

public:
	using size_type = std::size_t;

	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using rvalue_reference = value_type&&;

public:
	TypeVector(EntityTypeMatrix& matrix) : m_matrix(matrix) {}

	TypeVector& operator=(const TypeVector& rhs);

	// Element access
	value_type operator[](size_t index);
	const value_type operator[](size_t index) const;

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
	void clear();

	// Modifiers
	template <class T>
	void insert(const_iterator where, const ComponentVector<T>& components);
	template <class T>
	void insert(const_iterator where, ComponentVector<T>&& components);

	template <class T, class... Args>
	void emplace(const_iterator where, Args&&... args);

	void erase(const_iterator where);

	template <class T>
	void push_back(const ComponentVector<T>& components) { insert(end(), components); }
	template <class T>
	void push_back(ComponentVector<T>&& components) { insert(end(), std::move(components)); }

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
	while (tarFirst != tarLast) {
		onMismatch(*tarFirst);
		++tarFirst;
	}
}


template <class... Components>
struct SortedComponents {
	static constexpr size_t count = sizeof...(Components);
	static const std::vector<std::pair<std::type_index, size_t>> order;
};

template <class... Components>
const std::vector<std::pair<std::type_index, size_t>> SortedComponents<Components...>::order = [] {
	std::vector<std::pair<std::type_index, size_t>> arr = { std::pair<std::type_index, size_t>{ typeid(Components), 0 }... };
	for (auto i : Range(SortedComponents<Components...>::count))
		arr[i].second = i;
	std::stable_sort(arr.begin(), arr.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});
	return arr;
}();


//------------------------------------------------------------------------------
// EntityVector::value_type -- implementation
//------------------------------------------------------------------------------

template <class AsType>
decltype(auto) EntityVector::value_type::get(size_t index) {
	auto& vec = dynamic_cast<ComponentVector<AsType>&>(*(*m_matrix)[index]);
	return vec[m_index];
}

template <class AsType>
decltype(auto) EntityVector::value_type::get(size_t index) const {
	auto& vec = dynamic_cast<const ComponentVector<AsType>&>(*(*m_matrix)[index]);
	return vec[m_index];
}

template <class... Components>
EntityVector::value_type& EntityVector::value_type::assign_extend(const value_type& rhs, Components&&... components) {
	if (m_matrix && rhs.m_matrix) {
		const std::vector<bool>& mask = assign_auto_mask(rhs);
		assign_extra_mask([&](const auto& tar) { return mask[tar.second]; }, std::forward<Components>(components)...);
	}
	else if (m_matrix) {
		assign_extra_mask([&](const auto& tar) { return false; }, std::forward<Components>(components)...);
	}

	return *this;
}

template <class... Components>
EntityVector::value_type& EntityVector::value_type::assign_extend(value_type&& rhs, Components&&... components) {
	if (m_matrix && rhs.m_matrix) {
		const std::vector<bool>& mask = assign_auto_mask(rhs);
		assign_extra_mask([&](const auto& tar) { return mask[tar.second]; }, std::forward<Components>(components)...);
	}
	else if (m_matrix) {
		assign_extra_mask([&](const auto& tar) { return false; }, std::forward<Components>(components)...);
	}

	return *this;
}

template <class SkipPred>
EntityVector::value_type& EntityVector::value_type::assign_partial(const value_type& rhs, SkipPred skipPred) {
	if (m_matrix && rhs.m_matrix) {
		assign_partial(rhs, skipPred, int());
	}
	return *this;
}

template <class SkipPred>
EntityVector::value_type& EntityVector::value_type::assign_partial(value_type&& rhs, SkipPred skipPred) {
	if (m_matrix && rhs.m_matrix) {
		assign_partial(rhs, skipPred, int());
	}
	return *this;
}

template <class other_t>
const std::vector<bool>& EntityVector::value_type::assign_auto_mask(other_t&& rhs) {
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

template <class Pred, class... Components>
void EntityVector::value_type::assign_extra_mask(Pred pred, Components&&... components) {
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

	assign_recurse<0>(pairing, std::forward_as_tuple(std::forward<Components>(components)...));
}


template <class other_t, class SkipPred>
void EntityVector::value_type::assign_partial(other_t&& rhs, SkipPred skipPred, int) {
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

template <int Index, class... Components>
void EntityVector::value_type::assign_recurse(const std::array<size_t, sizeof...(Components)>& pairing, const std::tuple<Components&&...>& args) {
	if constexpr (Index < sizeof...(Components)) {
		size_t thisIndex = pairing[Index];
		if (thisIndex < std::numeric_limits<size_t>::max()) {
			using ComponentT = std::tuple_element_t<Index, std::tuple<Components&&...>>;
			auto& thisVector = dynamic_cast<ComponentVector<std::decay_t<ComponentT>>&>(*(*m_matrix)[thisIndex]);
			thisVector[m_index] = std::forward<ComponentT>(std::get<Index>(args));
		}
		assign_recurse<Index + 1>(pairing, args);
	}
}


//------------------------------------------------------------------------------
// EntityVector::generic_iterator -- implementation
//------------------------------------------------------------------------------

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator++() {
	++m_value.m_index;
	return *this;
}

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator--() {
	--m_value.m_index;
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
	m_value.m_index += n;
	return *this;
}

template <bool Const>
EntityVector::generic_iterator<Const>& EntityVector::generic_iterator<Const>::operator-=(size_t n) {
	m_value.m_index -= n;
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
			using ComponentT = std::tuple_element_t<Index, std::tuple<Components&&...>>;
			auto& thisVector = dynamic_cast<ComponentVector<std::decay_t<ComponentT>>&>(*m_matrix[thisIndex]);
			thisVector.Insert(where.get_index(), std::forward<ComponentT>(std::get<Index>(args)));
		}
		emplace_recurse<Index + 1>(where, pairing, args);
	}
}


//------------------------------------------------------------------------------
// TypeVector::value_type -- implementation
//------------------------------------------------------------------------------

template <class AsType>
AsType& TypeVector::value_type::get(size_t index) {
	auto& vec = dynamic_cast<ComponentVector<AsType>&>(*(*m_matrix)[m_index]);
	return vec[index];
}

template <class AsType>
const AsType& TypeVector::value_type::get(size_t index) const {
	auto& vec = dynamic_cast<const ComponentVector<AsType>&>(*(*m_matrix)[m_index]);
	return vec[index];
}

template <class AsType>
ComponentVector<AsType>& TypeVector::value_type::get_vector() {
	auto& vec = dynamic_cast<ComponentVector<AsType>&>(*(*m_matrix)[m_index]);
	return vec;
}

template <class AsType>
const ComponentVector<AsType>& TypeVector::value_type::get_vector() const {
	auto& vec = dynamic_cast<const ComponentVector<AsType>&>(*(*m_matrix)[m_index]);
	return vec;
}

//------------------------------------------------------------------------------
// TypeVector::generic_iterator -- implementation
//------------------------------------------------------------------------------

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator++() {
	++m_value.m_index;
	return *this;
}

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator--() {
	--m_value.m_index;
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

	m_value.m_index += n;
	return *this;
}

template <bool Const>
TypeVector::generic_iterator<Const>& TypeVector::generic_iterator<Const>::operator-=(size_t n) {
	m_value.m_index -= n;
	return *this;
}


//------------------------------------------------------------------------------
// TypeVector -- implementation
//------------------------------------------------------------------------------

template <class T>
void TypeVector::insert(const_iterator where, const ComponentVector<T>& components) {
	auto newContainer = std::make_unique<ComponentVector<T>>(components);
	newContainer->Resize(m_matrix.empty() ? 0 : m_matrix[0]->Size());
	m_matrix.push_back(std::move(newContainer));
	recompute_order();
}

template <class T>
void TypeVector::insert(const_iterator where, ComponentVector<T>&& components) {
	auto newContainer = std::make_unique<ComponentVector<T>>(std::move(components));
	newContainer->Resize(m_matrix.empty() ? 0 : m_matrix[0]->Size());
	m_matrix.push_back(std::move(newContainer));
	recompute_order();
}

template <class T, class... Args>
void TypeVector::emplace(const_iterator where, Args&&... args) {
	m_matrix.push_back(std::make_unique<ComponentVector<T>>(std::forward<Args>(args)...));
	recompute_order();
}


} // namespace inl::game