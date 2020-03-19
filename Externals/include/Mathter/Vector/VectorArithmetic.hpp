#pragma once

#include "VectorImpl.hpp"


namespace mathter {

//------------------------------------------------------------------------------
// Vector arithmetic
//------------------------------------------------------------------------------

/// <summary> Elementwise (Hadamard) vector product. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator*(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] * rhs.data[i];
		}
		return result;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::mul(lhs.simd, rhs.simd) };
	}
}
/// <summary> Elementwise vector division. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator/(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] / rhs.data[i];
		}
		return result;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::div(lhs.simd, rhs.simd) };
	}
}
/// <summary> Elementwise vector addition. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator+(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] + rhs.data[i];
		}
		return result;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::add(lhs.simd, rhs.simd) };
	}
}
/// <summary> Elementwise vector subtraction. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator-(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] - rhs.data[i];
		}
		return result;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::sub(lhs.simd, rhs.simd) };
	}
}

//------------------------------------------------------------------------------
// Vector assign arithmetic
//------------------------------------------------------------------------------

/// <summary> Elementwise (Hadamard) vector product. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator*=(Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] *= rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::mul(lhs.simd, rhs.simd);
	}
	return lhs;
}

/// <summary> Elementwise vector division. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator/=(Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] /= rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::div(lhs.simd, rhs.simd);
	}
	return lhs;
}

/// <summary> Elementwise vector addition. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator+=(Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] += rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::add(lhs.simd, rhs.simd);
	}
	return lhs;
}

/// <summary> Elementwise vector subtraction. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator-=(Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] -= rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::sub(lhs.simd, rhs.simd);
	}
	return lhs;
}

//------------------------------------------------------------------------------
// Scalar assign arithmetic
//------------------------------------------------------------------------------

/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed>& operator*=(Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] *= rhs;
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::mul(lhs.simd, rhs);
	}
	return lhs;
}

/// <summary> Scales the vector by 1/<paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed>& operator/=(Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] /= rhs;
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::div(lhs.simd, rhs);
	}
	return lhs;
}

/// <summary> Adds <paramref name="rhs"/> to each element of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed>& operator+=(Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] += rhs;
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::add(lhs.simd, rhs);
	}
	return lhs;
}

/// <summary> Subtracts <paramref name="rhs"/> from each element of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed>& operator-=(Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] -= rhs;
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		lhs.simd = SimdT::sub(lhs.simd, rhs);
	}
	return lhs;
}


//------------------------------------------------------------------------------
// Scalar arithmetic
//------------------------------------------------------------------------------

/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator*(const Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> copy(lhs);
		copy *= rhs;
		return copy;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::mul(lhs.simd, rhs) };
	}
}
/// <summary> Scales the vector by 1/<paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator/(const Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> copy(lhs);
		copy /= rhs;
		return copy;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::div(lhs.simd, rhs) };
	}
}
/// <summary> Adds <paramref name="rhs"/> to each element of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator+(const Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> copy(lhs);
		copy += rhs;
		return copy;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::add(lhs.simd, rhs) };
	}
}
/// <summary> Subtracts <paramref name="rhs"/> from each element of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator-(const Vector<T, Dim, Packed>& lhs, U rhs) {
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		Vector<T, Dim, Packed> copy(lhs);
		copy -= rhs;
		return copy;
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		return { Vector<T, Dim, Packed>::FromSimd, SimdT::sub(lhs.simd, rhs) };
	}
}


/// <summary> Scales vector by <paramref name="lhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator*(U lhs, const Vector<T, Dim, Packed>& rhs) { return rhs * lhs; }
/// <summary> Adds <paramref name="lhs"/> to all elements of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator+(U lhs, const Vector<T, Dim, Packed>& rhs) { return rhs + lhs; }
/// <summary> Makes a vector with <paramref name="lhs"/> as all elements, then subtracts <paramref name="rhs"> from it. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator-(U lhs, const Vector<T, Dim, Packed>& rhs) { return Vector<T, Dim, Packed>(lhs) - rhs; }
/// <summary> Makes a vector with <paramref name="lhs"/> as all elements, then divides it by <paramref name="rhs">. </summary>
template <class T, int Dim, bool Packed, class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
inline Vector<T, Dim, Packed> operator/(U lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> copy(lhs);
	copy /= rhs;
	return copy;
}


//------------------------------------------------------------------------------
// Extra
//------------------------------------------------------------------------------

/// <summary> Return (a*b)+c. Performs MAD or FMA if supported by target architecture. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> MultiplyAdd(const Vector<T, Dim, Packed>& a, const Vector<T, Dim, Packed>& b, const Vector<T, Dim, Packed>& c) {
	return a * b + c;
}

/// <summary> Negates all elements of the vector. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator-(const Vector<T, Dim, Packed>& arg) {
	return arg * T(-1);
}

/// <summary> Optional plus sign, leaves the vector as is. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator+(const Vector<T, Dim, Packed>& arg) {
	return arg;
}

//------------------------------------------------------------------------------
// Swizzle-vector
//------------------------------------------------------------------------------

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator*(const Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return v * decltype(v)(s);
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator/(const Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return v / decltype(v)(s);
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator+(const Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return v + decltype(v)(s);
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator-(const Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return v - decltype(v)(s);
}



template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator*(const Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return decltype(v)(s) * v;
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator/(const Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return decltype(v)(s) / v;
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator+(const Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return decltype(v)(s) + v;
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator-(const Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>> {
	return decltype(v)(s) - v;
}



template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator*=(Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>>& {
	return v *= decltype(v)(s);
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator/=(Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>>& {
	return v /= decltype(v)(s);
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator+=(Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>>& {
	return v += decltype(v)(s);
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator-=(Vector<T, Dim, Packed>& v, const Swizzle<VectorDataT, Indices...>& s) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Vector<T, Dim, Packed>>& {
	return v -= decltype(v)(s);
}



template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator*=(Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Swizzle<VectorDataT, Indices...>>& {
	return s = decltype(v)(s) * v;
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator/=(Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Swizzle<VectorDataT, Indices...>>& {
	return s = decltype(v)(s) / v;
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator+=(Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Swizzle<VectorDataT, Indices...>>& {
	return s = decltype(v)(s) + v;
}

template <class T, int Dim, bool Packed, class VectorDataT, int... Indices>
auto operator-=(Swizzle<VectorDataT, Indices...>& s, const Vector<T, Dim, Packed>& v) -> std::enable_if_t<Dim == sizeof...(Indices) && std::is_same_v<T, typename traits::VectorTraits<VectorDataT>::Type>, Swizzle<VectorDataT, Indices...>>& {
	return s = decltype(v)(s) - v;
}


//------------------------------------------------------------------------------
// Swizzle-swizzle
//------------------------------------------------------------------------------


template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator*(const Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	using V1 = Vector<typename traits::VectorTraits<VData1>::Type, traits::VectorTraits<VData1>::Dim, traits::VectorTraits<VData1>::Packed>;
	using V2 = Vector<typename traits::VectorTraits<VData2>::Type, traits::VectorTraits<VData2>::Dim, traits::VectorTraits<VData2>::Packed>;
	return V1(s1) * V2(s2);
}

template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator/(const Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	using V1 = Vector<typename traits::VectorTraits<VData1>::Type, traits::VectorTraits<VData1>::Dim, traits::VectorTraits<VData1>::Packed>;
	using V2 = Vector<typename traits::VectorTraits<VData2>::Type, traits::VectorTraits<VData2>::Dim, traits::VectorTraits<VData2>::Packed>;
	return V1(s1) / V2(s2);
}

template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator+(const Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	using V1 = Vector<typename traits::VectorTraits<VData1>::Type, traits::VectorTraits<VData1>::Dim, traits::VectorTraits<VData1>::Packed>;
	using V2 = Vector<typename traits::VectorTraits<VData2>::Type, traits::VectorTraits<VData2>::Dim, traits::VectorTraits<VData2>::Packed>;
	return V1(s1) + V2(s2);
}

template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator-(const Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	using V1 = Vector<typename traits::VectorTraits<VData1>::Type, traits::VectorTraits<VData1>::Dim, traits::VectorTraits<VData1>::Packed>;
	using V2 = Vector<typename traits::VectorTraits<VData2>::Type, traits::VectorTraits<VData2>::Dim, traits::VectorTraits<VData2>::Packed>;
	return V1(s1) - V2(s2);
}



template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator*=(Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	return s1 = s1 * s2;
}

template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator/=(Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	return s1 = s1 / s2;
}

template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator+=(Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	return s1 = s1 + s2;
}

template <class VData1, int... Indices1, class VData2, int... Indices2>
auto operator-=(Swizzle<VData1, Indices1...>& s1, const Swizzle<VData2, Indices2...>& s2) {
	return s1 = s1 - s2;
}

//------------------------------------------------------------------------------
// Swizzle-scalar
//------------------------------------------------------------------------------

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator*(const Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	return VectorT(lhs) * rhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator/(const Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	return VectorT(lhs) / rhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator+(const Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	return VectorT(lhs) + rhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator-(const Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	return VectorT(lhs) - rhs;
}



template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator*(U lhs, const Swizzle<VectorDataT, Indices...>& rhs) {
	return rhs * lhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator/(U lhs, const Swizzle<VectorDataT, Indices...>& rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	return lhs / VectorT(rhs);
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator+(U lhs, const Swizzle<VectorDataT, Indices...>& rhs) {
	return rhs + lhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto operator-(U lhs, const Swizzle<VectorDataT, Indices...>& rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	return lhs - VectorT(rhs);
}



template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto& operator*=(Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	lhs = VectorT(lhs) * rhs;
	return lhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto& operator/=(Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	lhs = VectorT(lhs) / rhs;
	return lhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto& operator+=(Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	lhs = VectorT(lhs) + rhs;
	return lhs;
}

template <class VectorDataT, int... Indices, class U, class = std::enable_if_t<std::is_convertible_v<U, typename traits::VectorTraits<VectorDataT>::Type>>>
auto& operator-=(Swizzle<VectorDataT, Indices...>& lhs, U rhs) {
	using VectorT = Vector<typename traits::VectorTraits<VectorDataT>::Type,
						   traits::VectorTraits<VectorDataT>::Dim,
						   traits::VectorTraits<VectorDataT>::Packed>;
	lhs = VectorT(lhs) - rhs;
	return lhs;
}



} // namespace mathter