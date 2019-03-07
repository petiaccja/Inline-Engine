#pragma once

#include <vector>

namespace inl::game {


template <class... Components>
class ComponentRange {
	template <bool IsConst>
	class iterator_base {
		using IteratorSet = std::conditional_t<IsConst,
											   std::tuple<typename std::vector<Components*>::const_iterator...>,
											   std::tuple<typename std::vector<Components*>::iterator...>>;
	public:
		iterator_base() = default;
		explicit iterator_base(IteratorSet iterators) : m_iterators(iterators) {}

		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = std::conditional_t<IsConst, std::tuple<const Components&...>, std::tuple<Components&...>>;
		using difference_type = ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		reference operator*() const { return *static_cast<ComponentT*>(m_it->second); }
		pointer operator->() const { return static_cast<ComponentT*>(m_it->second); }

		iterator_base& operator++() { return ++m_it, *this; }
		iterator_base& operator--() { return --m_it, *this; }
		iterator_base operator++(int) {
			iterator_base copy = (*this);
			++*this;
			return copy;
		}
		iterator_base operator--(int) {
			iterator_base copy = (*this);
			--*this;
			return copy;
		}

		bool operator==(const iterator_base& rhs) const { return m_it == rhs.m_it; }
		bool operator!=(const iterator_base& rhs) const { return m_it != rhs.m_it; }

	private:
		std::tuple<typename std::vector<Components*>::iterator...> m_iterators;
	};
	
public:
	ComponentRange(const std::vector<Components*>&... componentPointers);

};


template <class Component>
class ComponentRange<Component> {
public:
};


} // namespace inl::game