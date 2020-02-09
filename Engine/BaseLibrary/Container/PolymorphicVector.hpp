#pragma once

#include "TransformIterator.hpp"

#include <memory>
#include <vector>


namespace inl {

template <class T, template <class U> class Alloc = std::allocator>
class PolymorphicVector {
	struct Transformer {
		T& operator()(std::unique_ptr<T>& ptr) const {
			return *ptr;
		}
		const T& operator()(const std::unique_ptr<T>& ptr) const {
			return *ptr;
		}
	};
	template <class U>
	decltype(auto) transform(U&& arg) {
		return Transformer{}(arg);
	}

public:
	// Types
	using allocator_type = Alloc<std::unique_ptr<T>>;
	using const_pointer = const T*;
	using const_reference = const T&;
	using pointer = T*;
	using reference = T&;
	using size_type = std::size_t;
	using value_type = T;
	using underlying_container = std::vector<std::unique_ptr<T>, allocator_type>;

	using iterator = TransformIterator<typename underlying_container::iterator, Transformer>;
	using const_iterator = TransformIterator<typename underlying_container::const_iterator, Transformer>;
	using reverse_iterator = TransformIterator<typename underlying_container::reverse_iterator, Transformer>;
	using const_reverse_iterator = TransformIterator<typename underlying_container::const_reverse_iterator, Transformer>;

	// Construction
	PolymorphicVector() noexcept(noexcept(allocator_type()));
	explicit PolymorphicVector(const allocator_type& alloc) noexcept;
	template <class U>
	PolymorphicVector(size_type count, const U& value, const allocator_type& alloc = allocator_type());
	template <class InputIt>
	PolymorphicVector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type()) requires !std::is_convertible_v<InputIt, T>;
	PolymorphicVector(PolymorphicVector&& other) noexcept;
	PolymorphicVector(PolymorphicVector&& other, const allocator_type& alloc);
	template <class U>
	PolymorphicVector(std::initializer_list<U> init, const allocator_type& alloc = allocator_type());
	template <class... Items>
	PolymorphicVector(Items&&... items) requires std::conjunction_v<std::is_convertible<Items*, T*>...>;

	PolymorphicVector& operator=(PolymorphicVector&& other);
	template <class U>
	PolymorphicVector& operator=(std::initializer_list<U> ilist);

	template <class U>
	void assign(size_type count, const U& value);
	template <class InputIt>
	void assign(InputIt first, InputIt last);
	template <class U>
	void assign(std::initializer_list<U> ilist);

	allocator_type get_allocator() const;

	// Element access
	reference at(size_type pos);
	const_reference at(size_type pos) const;

	reference operator[](size_type index);
	const_reference operator[](size_type index) const;

	reference back();
	const_reference back() const;
	reference front();
	const_reference front() const;

	// Iterators
	[[nodiscard]] iterator begin();
	[[nodiscard]] iterator end();
	[[nodiscard]] const_iterator begin() const;
	[[nodiscard]] const_iterator end() const;
	[[nodiscard]] const_iterator cbegin() const;
	[[nodiscard]] const_iterator cend() const;
	[[nodiscard]] reverse_iterator rbegin();
	[[nodiscard]] reverse_iterator rend();
	[[nodiscard]] const_reverse_iterator rbegin() const;
	[[nodiscard]] const_reverse_iterator rend() const;
	[[nodiscard]] const_reverse_iterator crbegin() const;
	[[nodiscard]] const_reverse_iterator crend() const;

	// Capacity
	size_type capacity() const noexcept;
	bool empty() const noexcept;
	size_type max_size() const noexcept;
	void reserve(size_type count);
	void resize(size_type count);
	void shrink_to_fit();
	size_type size() const noexcept;

	// Modify
	void clear();
	template <class U, class... Args>
	iterator emplace(const_iterator pos, std::in_place_type_t<U>, Args&&... args);
	template <class U, class... Args>
	reference emplace_back(std::in_place_type_t<U>, Args&&... args);

	template <class U>
	iterator insert(const_iterator pos, U&& value);
	template <class U>
	iterator insert(const_iterator pos, size_type count, const U& value);
	template <class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last);
	template <class U>
	iterator insert(const_iterator pos, std::initializer_list<U> ilist);

	template <class U>
	void push_back(U&& value);

	void erase(const_iterator it);
	void erase(const_iterator first, const_iterator last);

	// Customized parts
	void swap_elements(const_iterator elem1, const_iterator elem2);

private:
	underlying_container m_items;
};


//------------------------------------------------------------------------------
// Constructors.
//------------------------------------------------------------------------------
template <class T, template <class U> class Alloc>
PolymorphicVector<T, Alloc>::PolymorphicVector() noexcept(noexcept(allocator_type())) {}

template <class T, template <class U> class Alloc>
PolymorphicVector<T, Alloc>::PolymorphicVector(const allocator_type& alloc) noexcept
	: m_items{ alloc } {}

template <class T, template <class U> class Alloc>
template <class U>
PolymorphicVector<T, Alloc>::PolymorphicVector(size_type count, const U& value, const allocator_type& alloc) : m_items{ alloc } {
	m_items.reserve(count);
	for (size_type i = 0; i < count; ++i) {
		m_items.push_back(std::make_unique<U>(value));
	}
}

template <class T, template <class U> class Alloc>
template <class InputIt>
PolymorphicVector<T, Alloc>::PolymorphicVector(InputIt first, InputIt last, const allocator_type& alloc) requires !std::is_convertible_v<InputIt, T>
: m_items{ alloc } {
	if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename InputIt::iterator_category>) {
		m_items.reserve(std::distance(first, last));
	}
	for (; first != last; ++first) {
		m_items.push_back(std::make_unique<std::decay_t<decltype(*first)>>(*first));
	}
}

template <class T, template <class U> class Alloc>
PolymorphicVector<T, Alloc>::PolymorphicVector(PolymorphicVector&& other) noexcept
	: m_items{ std::move(other.m_items) } {}

template <class T, template <class U> class Alloc>
PolymorphicVector<T, Alloc>::PolymorphicVector(PolymorphicVector&& other, const allocator_type& alloc)
	: m_items{ other.m_items, alloc } {}

template <class T, template <class U> class Alloc>
template <class U>
PolymorphicVector<T, Alloc>::PolymorphicVector(std::initializer_list<U> init, const allocator_type& alloc) : m_items{ alloc } {
	m_items.reserve(init.size());
	for (typename std::initializer_list<U>::reference item : init) {
		m_items.push_back(std::make_unique<U>(item));
	}
}

template <class T, template <class U> class Alloc>
template <class ... Items>
PolymorphicVector<T, Alloc>::PolymorphicVector(Items&&... items) requires std::conjunction_v<std::is_convertible<Items*, T*>...> {
	(..., m_items.push_back(std::make_unique<Items>(std::forward<Items>(items))));
}

//------------------------------------------------------------------------------
// Construct/assign.
//------------------------------------------------------------------------------

template <class T, template <class U> class Alloc>
PolymorphicVector<T, Alloc>& PolymorphicVector<T, Alloc>::operator=(PolymorphicVector&& other) {
	m_items = std::move(other.m_items);
	return *this;
}

template <class T, template <class U> class Alloc>
template <class U>
PolymorphicVector<T, Alloc>& PolymorphicVector<T, Alloc>::operator=(std::initializer_list<U> ilist) {
	m_items.clear();
	m_items.reserve(ilist.size());
	for (typename std::initializer_list<U>::reference item : ilist) {
		m_items.push_back(std::make_unique<U>(item));
	}
	return *this;
}

template <class T, template <class U> class Alloc>
template <class U>
void PolymorphicVector<T, Alloc>::assign(size_type count, const U& value) {
	m_items.clear();
	m_items.reserve(count);
	for (size_type i = 0; i < count; ++i) {
		m_items.push_back(std::make_unique<U>(value));
	}
}

template <class T, template <class U> class Alloc>
template <class InputIt>
void PolymorphicVector<T, Alloc>::assign(InputIt first, InputIt last) {
	m_items.clear();
	if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename InputIt::iterator_category>) {
		m_items.reserve(std::distance(first, last));
	}
	for (; first != last; ++first) {
		m_items.push_back(std::make_unique<std::decay_t<decltype(*first)>>(*first));
	}
}

template <class T, template <class U> class Alloc>
template <class U>
void PolymorphicVector<T, Alloc>::assign(std::initializer_list<U> ilist) {
	m_items.clear();
	m_items.reserve(ilist.size());
	for (typename std::initializer_list<U>::reference item : ilist) {
		m_items.push_back(std::make_unique<U>(item));
	}
	return *this;
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::allocator_type PolymorphicVector<T, Alloc>::get_allocator() const {
	return m_items.get_allocator();
}

//------------------------------------------------------------------------------
// Element access.
//------------------------------------------------------------------------------

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::reference PolymorphicVector<T, Alloc>::at(size_type pos) {
	return transform(m_items.at(pos));
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reference PolymorphicVector<T, Alloc>::at(size_type pos) const {
	return transform(m_items.at(pos));
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::reference PolymorphicVector<T, Alloc>::operator[](size_type index) {
	return transform(m_items[index]);
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reference PolymorphicVector<T, Alloc>::operator[](size_type index) const {
	return transform(m_items[index]);
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::reference PolymorphicVector<T, Alloc>::back() {
	return transform(m_items.back());
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reference PolymorphicVector<T, Alloc>::back() const {
	return transform(m_items.back());
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::reference PolymorphicVector<T, Alloc>::front() {
	return transform(m_items.front());
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reference PolymorphicVector<T, Alloc>::front() const {
	return transform(m_items.front());
}


//------------------------------------------------------------------------------
// Iterators.
//------------------------------------------------------------------------------

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::begin() {
	return iterator{ m_items.begin() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::end() {
	return iterator{ m_items.end() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_iterator PolymorphicVector<T, Alloc>::begin() const {
	return const_iterator{ m_items.begin() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_iterator PolymorphicVector<T, Alloc>::end() const {
	return const_iterator{ m_items.end() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_iterator PolymorphicVector<T, Alloc>::cbegin() const {
	return const_iterator{ m_items.cbegin() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_iterator PolymorphicVector<T, Alloc>::cend() const {
	return const_iterator{ m_items.cend() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::reverse_iterator PolymorphicVector<T, Alloc>::rbegin() {
	return reverse_iterator{ m_items.rbegin() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::reverse_iterator PolymorphicVector<T, Alloc>::rend() {
	return reverse_iterator{ m_items.rend() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reverse_iterator PolymorphicVector<T, Alloc>::rbegin() const {
	return const_reverse_iterator{ m_items.rbegin() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reverse_iterator PolymorphicVector<T, Alloc>::rend() const {
	return const_reverse_iterator{ m_items.rend() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reverse_iterator PolymorphicVector<T, Alloc>::crbegin() const {
	return const_reverse_iterator{ m_items.crbegin() };
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::const_reverse_iterator PolymorphicVector<T, Alloc>::crend() const {
	return const_reverse_iterator{ m_items.crend() };
}


//------------------------------------------------------------------------------
// Capacity.
//------------------------------------------------------------------------------

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::size_type PolymorphicVector<T, Alloc>::capacity() const noexcept {
	return m_items.capacity();
}

template <class T, template <class U> class Alloc>
bool PolymorphicVector<T, Alloc>::empty() const noexcept {
	return m_items.empty();
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::size_type PolymorphicVector<T, Alloc>::max_size() const noexcept {
	return m_items.max_size();
}

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::reserve(size_type count) {
	return m_items.reserve();
}

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::resize(size_type count) {
	return m_items.resize();
}

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::shrink_to_fit() {
	return m_items.shrink_to_fit();
}

template <class T, template <class U> class Alloc>
typename PolymorphicVector<T, Alloc>::size_type PolymorphicVector<T, Alloc>::size() const noexcept {
	return m_items.size();
}


//------------------------------------------------------------------------------
// Modify.
//------------------------------------------------------------------------------

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::clear() {
	m_items.clear();
}

template <class T, template <class U> class Alloc>
template <class U, class... Args>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::emplace(const_iterator pos, std::in_place_type_t<U>, Args&&... args) {
	auto index = pos - begin();
	m_items.insert(m_items.begin() + pos, std::make_unique<U>(std::forward<Args>(args)...));
}

template <class T, template <class U> class Alloc>
template <class U, class... Args>
typename PolymorphicVector<T, Alloc>::reference PolymorphicVector<T, Alloc>::emplace_back(std::in_place_type_t<U>, Args&&... args) {
	m_items.push_back(std::make_unique<U>(std::forward<Args>(args)...));
}

template <class T, template <class U> class Alloc>
template <class U>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::insert(const_iterator pos, U&& value) {
	auto index = pos - begin();
	m_items.insert(m_items.begin() + index, std::make_unique<U>(std::forward<U>(value)));
}

template <class T, template <class U> class Alloc>
template <class U>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::insert(const_iterator pos, size_type count, const U& value) {
	auto index = pos - begin();
	m_items.insert(m_items.begin() + index, {});
	for (auto it = m_items.begin() + index; it < m_items.begin() + index + count; ++it) {
		*it = std::make_unique<U>(value);
	}
}

template <class T, template <class U> class Alloc>
template <class InputIt>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::insert(const_iterator pos, InputIt first, InputIt last) {
	auto index = pos - begin();
	auto itemsPos = m_items.begin() + index;
	for (; first != last; ++last, ++itemsPos) {
		m_items.insert(itemsPos, std::make_unique<std::decay_t<decltype(*first)>>(*first));
	}
}

template <class T, template <class U> class Alloc>
template <class U>
typename PolymorphicVector<T, Alloc>::iterator PolymorphicVector<T, Alloc>::insert(const_iterator pos, std::initializer_list<U> ilist) {
	auto index = pos - begin();
	auto count = ilist.size();
	m_items.insert(m_items.begin() + index, {});
	auto ilistIt = ilist.begin();
	for (auto it = m_items.begin() + index; it < m_items.begin() + index + count; ++it) {
		*it = std::make_unique<U>(*(ilistIt++));
	}
}

template <class T, template <class U> class Alloc>
template <class U>
void PolymorphicVector<T, Alloc>::push_back(U&& value) {
	m_items.push_back(std::make_unique<U>(std::forward<U>(value)));
}

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::erase(const_iterator it) {
	auto index = it - begin();
	auto itemsPos = m_items.begin() + index;
	m_items.erase(itemsPos);
}

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::erase(const_iterator first, const_iterator last) {
	auto indexFirst = first - begin();
	auto indexLast = last - begin();
	m_items.erase(m_items.begin() + indexFirst, m_items.begin() + indexLast);
}


//------------------------------------------------------------------------------
// Custom.
//------------------------------------------------------------------------------

template <class T, template <class U> class Alloc>
void PolymorphicVector<T, Alloc>::swap_elements(const_iterator elem1, const_iterator elem2) {
	auto index1 = elem1 - begin();
	auto index2 = elem2 - begin();
	std::swap(m_items[index1], m_items[index2]);
}


} // namespace inl