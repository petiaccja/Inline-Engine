#pragma once

#include "VectorImpl.hpp"


namespace mathter {

/// <summary> Elementwise (Hadamard) vector product. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator*(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> result;
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] * rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		result.simd = SimdT::mul(lhs.simd, rhs.simd);
	}
	return result;
}
/// <summary> Elementwise vector division. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator/(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> result;
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] / rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		result.simd = SimdT::div(lhs.simd, rhs.simd);
	}
	return result;
}
/// <summary> Elementwise vector addition. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator+(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> result;
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] + rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		result.simd = SimdT::add(lhs.simd, rhs.simd);
	}
	return result;
}
/// <summary> Elementwise vector subtraction. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator-(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> result;
	if constexpr (!traits::HasSimd<Vector<T, Dim, Packed>>::value) {
		for (int i = 0; i < Dim; ++i) {
			result[i] = lhs.data[i] - rhs.data[i];
		}
	}
	else {
		using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
		result.simd = SimdT::sub(lhs.simd, rhs.simd);
	}
	return result;
}

// Vector assign arithmetic
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

// Scalar assign arithmetic
/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator*=(Vector<T, Dim, Packed>& lhs, T rhs) {
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
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator/=(Vector<T, Dim, Packed>& lhs, T rhs) {
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
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator+=(Vector<T, Dim, Packed>& lhs, T rhs) {
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
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed>& operator-=(Vector<T, Dim, Packed>& lhs, T rhs) {
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


/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator*(const Vector<T, Dim, Packed>& lhs, T rhs) {
	Vector<T, Dim, Packed> copy(lhs);
	copy *= rhs;
	return copy;
}
/// <summary> Scales the vector by 1/<paramref name="rhs"/>. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator/(const Vector<T, Dim, Packed>& lhs, T rhs) {
	Vector<T, Dim, Packed> copy(lhs);
	copy /= rhs;
	return copy;
}
/// <summary> Adds <paramref name="rhs"/> to each element of the vector. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator+(const Vector<T, Dim, Packed>& lhs, T rhs) {
	Vector<T, Dim, Packed> copy(lhs);
	copy += rhs;
	return copy;
}
/// <summary> Subtracts <paramref name="rhs"/> from each element of the vector. </summary>
template <class T, int Dim, bool Packed>
inline Vector<T, Dim, Packed> operator-(const Vector<T, Dim, Packed>& lhs, T rhs) {
	Vector<T, Dim, Packed> copy(lhs);
	copy -= rhs;
	return copy;
}


/// <summary> Scales vector by <paramref name="lhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator*(U lhs, const Vector<T, Dim, Packed>& rhs) { return rhs * (T)lhs; }
/// <summary> Adds <paramref name="lhs"/> to all elements of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator+(U lhs, const Vector<T, Dim, Packed>& rhs) { return rhs + (T)lhs; }
/// <summary> Makes a vector with <paramref name="lhs"/> as all elements, then subtracts <paramref name="rhs"> from it. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator-(U lhs, const Vector<T, Dim, Packed>& rhs) { return Vector<T, Dim, Packed>(lhs) - rhs; }
/// <summary> Makes a vector with <paramref name="lhs"/> as all elements, then divides it by <paramref name="rhs">. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator/(U lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> copy(lhs);
	copy /= rhs;
	return copy;
}


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
// Swizzle ops
//------------------------------------------------------------------------------

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator*(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v * decltype(v)(s);
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator/(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v / decltype(v)(s);
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator+(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v + decltype(v)(s);
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator-(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v - decltype(v)(s);
}



template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator*(const Swizzle<T, Indices...>& s, const Vector<T, Dim, Packed>& v) {
	return decltype(v)(s) * v;
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator/(const Swizzle<T, Indices...>& s, const Vector<T, Dim, Packed>& v) {
	return decltype(v)(s) / v;
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator+(const Swizzle<T, Indices...>& s, const Vector<T, Dim, Packed>& v) {
	return decltype(v)(s) + v;
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>> operator-(const Swizzle<T, Indices...>& s, const Vector<T, Dim, Packed>& v) {
	return decltype(v)(s) - v;
}



template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>>& operator*=(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v *= decltype(v)(s);
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>>& operator/=(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v /= decltype(v)(s);
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>>& operator+=(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v += decltype(v)(s);
}

template <class T, int Dim, bool Packed, int... Indices>
std::enable_if_t<Dim == sizeof...(Indices), Vector<T, Dim, Packed>>& operator-=(const Vector<T, Dim, Packed>& v, const Swizzle<T, Indices...>& s) {
	return v -= decltype(v)(s);
}



} // namespace mathter