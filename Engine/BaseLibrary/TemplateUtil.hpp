#pragma once

#include <type_traits>

namespace exc {


template<typename ...AnyT>
struct all {};

template <>
struct all<> : public std::true_type {};

template<typename HeadT, typename ...TailT>
struct all<HeadT, TailT...> : public std::integral_constant<bool, HeadT::value && all<TailT...>::value> {};


} // namespace exc
