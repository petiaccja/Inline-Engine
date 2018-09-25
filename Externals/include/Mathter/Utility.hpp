#pragma once

#include <type_traits>


namespace mathter {

template <class T, int Dim, bool Packed>
class Vector;

//------------------------------------------------------------------------------
// Public utility stuff
//------------------------------------------------------------------------------


// Common mathematical constants
template <class Scalar>
class Constants {
public:
	static constexpr Scalar Pi = (Scalar)3.1415926535897932384626433832795028841971693993751;
	static constexpr Scalar PiHalf = (Scalar)1.5707963267948966192313216916397514420985846996876;
	static constexpr Scalar PiFourth = (Scalar)0.78539816339744830961566084581987572104929234984378;
	static constexpr Scalar E = (Scalar)2.7182818284590452353602874713526624977572470937;
	static constexpr Scalar Sqrt2 = (Scalar)1.4142135623730950488016887242096980785696718753769;
	static constexpr Scalar SqrtHalf = (Scalar)0.70710678118654752440084436210484903928483593768847;
};


// Radians and degrees
template <class Scalar>
auto Rad2Deg(Scalar rad) {
	using ComputeT = typename std::conditional<std::is_floating_point<Scalar>::value, Scalar, long double>::type;
	return rad / Constants<ComputeT>::Pi * ComputeT(180);
}

template <class Scalar>
auto Deg2Rad(Scalar deg) {
	using ComputeT = typename std::conditional<std::is_floating_point<Scalar>::value, Scalar, long double>::type;
	return deg / ComputeT(180) * Constants<ComputeT>::Pi;
}


// Clamp and saturate
template <class Scalar>
Scalar Clamp(Scalar arg, Scalar lower, Scalar upper) {
	return std::max(lower, std::min(upper, arg));
}

template <class T, int Dim, bool Packed>
Vector<T, Dim, Packed> Clamp(const Vector<T, Dim, Packed>& arg, T lower, T upper);

template <class Scalar>
Scalar Saturate(Scalar arg) {
	return Clamp(arg, Scalar(0), Scalar(1));
}

template <class T, int Dim, bool Packed>
Vector<T, Dim, Packed> Saturate(const Vector<T, Dim, Packed>& arg);





//------------------------------------------------------------------------------
// Internal utility stuff.
//------------------------------------------------------------------------------
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




#include "Vector.hpp"

// Implementations of vector clamp functions.
namespace mathter {

template <class T, int Dim, bool Packed>
Vector<T, Dim, Packed> Clamp(const Vector<T, Dim, Packed>& arg, T lower, T upper) {
	Vector<T, Dim, Packed> ret;
	for (int i = 0; i < arg.Dimension(); ++i) {
		ret(i) = Clamp(arg(i), lower, upper);
	}
	return ret;
}

template <class T, int Dim, bool Packed>
Vector<T, Dim, Packed> Saturate(const Vector<T, Dim, Packed>& arg) {
	return Clamp(arg, T(0), T(1));
}

} // namespace mathter