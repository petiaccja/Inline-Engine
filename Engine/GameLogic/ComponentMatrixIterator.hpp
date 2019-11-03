#include <type_traits>
#include <BaseLibrary/TemplateUtil.hpp>
#include <cassert>


namespace inl::game {


template <bool Const>
class EntityReference {
	template <bool ItConst>
	friend class ComponentMatrixIterator;
	friend class ComponentMatrix;

public:
	EntityReference(templ::add_const_conditional_t<ComponentMatrix, Const>& componentMatrix, size_t index)
		: m_componentMatrix(componentMatrix), m_index(index) {}

	template <class Component>
	templ::add_const_conditional_t<Component, Const>& GetComponent(size_t index) const;

	size_t GetComponentCount() const;

	EntityReference& operator=(const EntityReference& rhs);

private:
	templ::add_const_conditional_t<ComponentMatrix, Const>& m_componentMatrix;
	size_t m_index;
};


template <bool Const>
class ComponentMatrixIterator {
public:
	friend class ComponentMatrix;

	ComponentMatrixIterator(templ::add_const_conditional_t<ComponentMatrix, Const>& componentMatrix, size_t index)
		: m_ref(componentMatrix, index) {}

	template <class Q = std::enable_if_t<!Const, ComponentMatrixIterator<Const>>>
	operator Q() const {
		return Q{ m_ref.m_componentMatrix, m_ref.m_index };
	}

	EntityReference<Const>& operator*() const { return m_ref; }
	EntityReference<Const>* operator->() const { return &m_ref; }

	ComponentMatrixIterator& operator++();
	ComponentMatrixIterator& operator--();
	ComponentMatrixIterator operator++(int);
	ComponentMatrixIterator operator--(int);
	ComponentMatrixIterator& operator+=(size_t n);
	ComponentMatrixIterator& operator-=(size_t n);
	friend ComponentMatrixIterator operator+(ComponentMatrixIterator lhs, size_t n) { return lhs += n; }
	friend ComponentMatrixIterator operator+(size_t n, ComponentMatrixIterator rhs) { return rhs += n; }
	friend ComponentMatrixIterator operator-(ComponentMatrixIterator lhs, size_t n) { return lhs -= n; }
	friend size_t operator-(const ComponentMatrixIterator& lhs, const ComponentMatrixIterator& rhs) { return lhs.m_ref.index - rhs.m_ref.index; }

	friend auto operator<=>(const ComponentMatrixIterator& lhs, const ComponentMatrixIterator& rhs) { return lhs.m_ref.index <=> rhs.m_ref.index; }

private:
	EntityReference<Const> m_ref;
};




template <bool Const>
template <class Component>
templ::add_const_conditional_t<Component, Const>& EntityReference<Const>::GetComponent(size_t index) const {
	auto& container = *m_componentMatrix.m_componentVectors[index];
	return dynamic_cast<templ::add_const_conditional_t<Component, Const>&>(container)[m_index];
}


template <bool Const>
size_t EntityReference<Const>::GetComponentCount() const {
	return m_componentMatrix.m_componentVectors.size();
}


template <bool Const>
EntityReference<Const>& EntityReference<Const>::operator=(const EntityReference& rhs) {
	assert(false);
	return *this;
}


template <bool Const>
ComponentMatrixIterator<Const>& ComponentMatrixIterator<Const>::operator++() {
	++m_ref.index;
	return *this;
}


template <bool Const>
ComponentMatrixIterator<Const>& ComponentMatrixIterator<Const>::operator--() {
	--m_ref.index;
	return *this;
}


template <bool Const>
ComponentMatrixIterator<Const> ComponentMatrixIterator<Const>::operator++(int) {
	auto cpy = *this;
	++m_ref.index;
	return cpy;
}


template <bool Const>
ComponentMatrixIterator<Const> ComponentMatrixIterator<Const>::operator--(int) {
	auto cpy = *this;
	--m_ref.index;
	return cpy;
}


template <bool Const>
ComponentMatrixIterator<Const>& ComponentMatrixIterator<Const>::operator+=(size_t n) {
	m_ref.index += n;
	return *this;
}


template <bool Const>
ComponentMatrixIterator<Const>& ComponentMatrixIterator<Const>::operator-=(size_t n) {
	m_ref.index -= n;
	return *this;
}





} // namespace inl::game