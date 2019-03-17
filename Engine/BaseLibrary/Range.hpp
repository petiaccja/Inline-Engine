#pragma once

#include <cstdint>
#include <iterator>
#include <cassert>


namespace inl {



template <class T>
class RangeHelper {
public:
	class iterator {
		friend class RangeHelper;
		iterator(T value, T step) : value(value), step(step) {}
	public:
		iterator() : value(std::numeric_limits<T>::lowest()) {}

		using value_type = T;
		using difference_type = ptrdiff_t;
		using reference = T&;
		using pointer = T*;
		using iterator_category = std::forward_iterator_tag;

		void operator++() {
			value += step;
		}
		T operator*() const {
			return value;
		}
		bool operator==(const iterator& rhs) const {
			return value == rhs.value;
		}
		bool operator!=(const iterator& rhs) const {
			return !(*this == rhs);
		}
	private:
		T value;
		T step;
	};

	RangeHelper(T first, T last, T step) : first(first), step(step) {
		assert((last >= first && std::is_unsigned_v<T>) || std::is_signed_v<T>);
		T diff = last-first;
		T sign = (step >= 0) - (step < 0);
		T rounded = ((diff + step - sign) / step) * step;
		this->last = first + rounded;
	}

	iterator begin() const { return iterator(first, step); }
	iterator end() const { return iterator(last, step); }
	iterator cbegin() const { return iterator(first, step); }
	iterator cend() const { return iterator(last, step); }
private:
	T first, last, step;
};


template <class T>
RangeHelper<T> Range(T first, T last, T step) {
	static_assert(std::is_arithmetic_v<T>, "Can only be used for arithmetic types.");
	return RangeHelper<T>(first, last, step);
}

template <class T>
RangeHelper<T> Range(T first, T last) {
	static_assert(std::is_arithmetic_v<T>, "Can only be used for arithmetic types.");
	T step = last >= first ? T(1) : T(-1);
	return Range(first, last, step);
}

template <class T>
RangeHelper<T> Range(T last) {
	static_assert(std::is_arithmetic_v<T>, "Can only be used for arithmetic types.");
	T first = T(0);
	T step = last >= first ? T(1) : T(-1);
	return Range(first, last, step);
}




}
