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
	template <OS&(*P)(OS&, const T&) = &OS::operator<< >
	static constexpr bool check(int) { return true; }

	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check(1);
};
template <class OS>
struct is_printable<void, OS> {
	static constexpr bool value = false;
};

// can be read from istream
template <class T, class IS = std::istream>
struct is_readable {
private:
	template <IS&(*P)(IS&, const T&) = &IS::operator>> >
	static constexpr bool check(int) { return true; }

	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check(1);
};

template <class IS>
struct is_readable<void, IS> {
	static constexpr bool value = false;
};



// can be compared with == 
template <class T>
struct is_equality_comparable {
private:
	template <class U = decltype(bool(*(const T*)nullptr == *(const T*)nullptr))>
	static constexpr bool check(int) { return true; }

	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check(1);
};


// can be compared with <
template <class T>
struct is_less_comparable {
private:
	template <class U = decltype(bool(*(const T*)nullptr < *(const T*)nullptr))>
	static constexpr bool check(int) { return true; }

	static constexpr bool check(...) { return false; }
public:
	static constexpr bool value = check(1);
};




} // namespace inl::templ
