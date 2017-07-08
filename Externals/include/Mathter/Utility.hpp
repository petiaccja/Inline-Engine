#pragma once


namespace mathter {
namespace impl {



template <class T>
constexpr T ConstexprExp10(int exponent) {
	return exponent == 0 ? T(1) : T(10) * ConstexprExp10<T>(exponent - 1);
}

template <class T>
constexpr T ConstexprAbs(T arg) {
	return arg >= T(0) ? arg : -arg;
}



} // namespace impl
} // namespace mathter