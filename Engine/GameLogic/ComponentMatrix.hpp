#include "ComponentScheme.hpp"
#include "ComponentVector.hpp"

#include <BaseLibrary/Range.hpp>
#include <BaseLibrary/TemplateUtil.hpp>

namespace inl::game {


//------------------------------------------------------------------------------
// Notes
//------------------------------------------------------------------------------

// BUT WHY WAS THIS ALL NEEDED?
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



//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//using EntityTypeMatrix = std::vector<std::unique_ptr<ComponentVectorBase>>;
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
		template <class SourceT = std::enable_if_t<Const, generic_reference<!Const>>>
		generic_reference(const SourceT& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		template <class AsType>
		templ::add_const_conditional_t<AsType, Const>& get(size_t index) const;
		size_t size() const;
		std::type_index get_type(size_t index) const;
		generic_reference& operator=(const generic_reference<true>& rhs);

		generic_reference* operator->() { return this; }

	private:
		void assign_same(const generic_reference<true>& rhs);
		void assign_auto(const generic_reference<true>& rhs);

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
		template <class SourceT = std::enable_if_t<Const, generic_iterator<!Const>>>
		generic_iterator(const SourceT& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

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
	void insert(const_iterator where, const_reference entity);
	template <class... Components>
	void emplace(const_iterator where, Components&&... components);
	void erase(const_iterator where);

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
		template <class SourceT = std::enable_if_t<Const, generic_reference<!Const>>>
		generic_reference(const SourceT& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

		template <class const_reference_t = std::enable_if_t<!Const, generic_reference<Const>>>
		operator const_reference_t() const { return const_reference_t{ m_index }; }

		template <class AsType>
		templ::add_const_conditional_t<AsType, Const>& get(size_t index) const;
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
		template <class SourceT = std::enable_if_t<Const, generic_iterator<!Const>>>
		generic_iterator(const SourceT& rhs) : m_index(rhs.m_index), m_matrix(rhs.m_matrix) {}

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

	template <class T, class... Args>
	void emplace(const_iterator where, Args&&... args);

	void erase(const_iterator where);

	auto& type_order() const { return m_orderedTypes; }

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
	if (m_matrix == rhs.m_matrix) {
		assign_same(rhs);
	}
	else {
		assign_auto(rhs);
	}
	return *this;
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
void EntityVector::generic_reference<Const>::assign_auto(const generic_reference<true>& rhs) {
	if (!m_matrix || !rhs.m_matrix) {
		return;
	}

	auto& lhsOrder = m_matrix->m_parent.types.type_order();
	auto& rhsOrder = rhs.m_matrix->m_parent.types.type_order();

	auto lhsIt = lhsOrder.begin();
	auto rhsIt = rhsOrder.begin();
	auto lhsEnd = lhsOrder.end();
	auto rhsEnd = rhsOrder.end();

	while (lhsIt != lhsEnd && rhsIt != rhsEnd) {
		if (lhsIt->first < rhsIt->first) {
			++lhsIt;
		}
		else if (lhsIt->first > rhsIt->first) {
			++rhsIt;
		}
		else {
			auto& lhsVec = (*m_matrix)[lhsIt->second];
			auto& rhsVec = (*rhs.m_matrix)[rhsIt->second];
			lhsVec->Copy(m_index, *rhsVec, rhs.m_index);
			++lhsIt;
			++rhsIt;
		}
	}
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



template <class... Components>
void EntityVector::emplace(const_iterator where, Components&&... components) {
	auto index = where.get_index();

	const auto& thisOrder = m_matrix.m_parent.types.type_order();
	const auto& argOrder = SortedComponents<Components...>::order;

	auto thisIt = thisOrder.begin();
	auto argIt = argOrder.begin();
	auto thisEnd = thisOrder.end();
	auto argEnd = argOrder.end();

	std::array<size_t, sizeof...(Components)> pairing;
	memset(&pairing, 0xFF, sizeof(pairing));

	while (thisIt != thisEnd && argIt != argEnd) {
		if (thisIt->first < argIt->first) {
			m_matrix[thisIt->second]->InsertDefault(where.get_index());
			++thisIt;
		}
		else if (thisIt->first > argIt->first) {
			++argIt;
		}
		else {
			size_t thisIndex = thisIt->second;
			size_t argIndex = argIt->second;
			pairing[argIndex] = thisIndex;

			++thisIt;
			++argIt;
		}
	}

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

template <class T, class... Args>
void TypeVector::emplace(const_iterator where, Args&&... args) {
	m_matrix.push_back(std::make_unique<_ComponentVector<T>>(std::forward<Args>(args)...));
	recompute_order();
}


} // namespace inl::game