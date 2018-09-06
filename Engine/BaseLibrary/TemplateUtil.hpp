#pragma once

#include <type_traits>
#include <iostream>

namespace inl::templ {


// all, conjunction
template<class ...AnyT>
struct all;

template <>
struct all<> : public std::true_type {};

template <class HeadT, class ...TailT>
struct all<HeadT, TailT...> : public std::integral_constant<bool, HeadT::value && all<TailT...>::value> {};


// any, disjunction
template <class ...AnyT>
struct any;

template <>
struct any<> : public std::false_type {};

template <class HeadT, class... TailT>
struct any<HeadT, TailT...> : public std::integral_constant<bool, HeadT::value || any<TailT...>::value> {};


// can be written to ostream
template <class T, class OS = std::ostream>
struct is_printable {
private:
	template <class U, class R>
	struct Helper {};

	template <class U>
	static constexpr bool check(Helper<U, decltype(*(OS*)nullptr << *(const std::decay_t<U>*)nullptr)>*) { return true; }

	template <class U>
	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check<T>(0);
};

template <class OS>
struct is_printable<void, OS> {
	static constexpr bool value = false;
};

// can be read from istream
template <class T, class IS = std::istream>
struct is_readable {
private:
	template <class U, class R>
	struct Helper {};

	template <class U>
	static constexpr bool check(Helper<U, decltype(*(IS*)nullptr >> (*(std::decay_t<U>*)nullptr))>*) {
		return !std::is_const_v<std::remove_reference_t<U>> && !std::is_rvalue_reference_v<U>; 
	}

	template <class U>
	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check<T>(0);
};

template <class IS>
struct is_readable<void, IS> {
	static constexpr bool value = false;
};



// can be compared with == 
template <class T>
struct is_equality_comparable {
private:
	template <class U>
	struct Helper {};

	template <class U>
	static constexpr bool check(Helper<decltype(*(U*)nullptr == *(U*)nullptr)>*) { return true; }

	template <class U>
	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check<T>(0);
};


// can be compared with <
template <class T>
struct is_less_comparable {
private:
	template <class U>
	struct Helper {};

	template <class U>
	static constexpr bool check(Helper<decltype(*(U*)nullptr < *(U*)nullptr)>*) { return true; }

	template <class U>
	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check<T>(0);
};



} // namespace inl::templ
