#pragma once

#include <vector>
#include <algorithm>


namespace inl::game {

template <class T, class Alloc = std::allocator<T>>
class ContiguousVector : private std::vector<T, Alloc> {
	using VectorT = std::vector<T, Alloc>;

public:
	using VectorT::allocator_type;
	using VectorT::const_iterator;
	using VectorT::const_pointer;
	using VectorT::const_reference;
	using VectorT::const_reverse_iterator;
	using VectorT::iterator;
	using VectorT::pointer;
	using VectorT::reference;
	using VectorT::reverse_iterator;
	using VectorT::value_type;
	using VectorT::size_type;

	using VectorT::VectorT;
	using VectorT::operator=;
	using VectorT::assign;
	using VectorT::get_allocator;

	using VectorT::at;
	using VectorT::operator[];
	using VectorT::data;

	using VectorT::begin;
	using VectorT::cbegin;
	using VectorT::cend;
	using VectorT::crbegin;
	using VectorT::crend;
	using VectorT::end;
	using VectorT::rbegin;
	using VectorT::rend;

	using VectorT::resize;
	using VectorT::capacity;
	using VectorT::empty;
	using VectorT::max_size;
	using VectorT::reserve;
	using VectorT::shrink_to_fit;
	using VectorT::size;

	using VectorT::clear;
	using VectorT::emplace_back;
	using VectorT::push_back;
	void erase(const_iterator it);
	void erase(const_iterator first, const_iterator last);
	void swap_elements(const_iterator elem1, const_iterator elem2);
};


template <class T, class Alloc>
void ContiguousVector<T, Alloc>::erase(const_iterator it) {
	iterator last = --end(); // Container must not be empty to remove from it.
	const_cast<T&>(*it) = std::move(*last);
	VectorT::pop_back();
}

template <class T, class Alloc>
void ContiguousVector<T, Alloc>::erase(const_iterator first, const_iterator last) {
	size_type count = last - first;
	const_iterator endIt = cend();
	const_iterator firstToMove = endIt - count;

	firstToMove = std::max(last, firstToMove);

	for (; firstToMove != endIt; ++firstToMove, ++first) {
		const_cast<T&>(*first) = std::move(const_cast<T&>(*last));
	}

	resize(size() - count);
}

template <class T, class Alloc>
void ContiguousVector<T, Alloc>::swap_elements(const_iterator elem1, const_iterator elem2) {
	std::swap(const_cast<T&>(*elem1), const_cast<T&>(*elem2));
}



} // namespace inl::game