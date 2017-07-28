#pragma once


namespace mathter {


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


template <class Scalar>
Scalar Rad2Deg(Scalar rad) {
	return rad / Constants<Scalar>::Pi * Scalar(180);
}

template <class Scalar>
Scalar Deg2Rad(Scalar deg) {
	return deg / Scalar(180) * Constants<Scalar>::Pi;
}




// Internal utility stuff.
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