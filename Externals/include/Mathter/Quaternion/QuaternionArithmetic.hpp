#pragma once

#include "QuaternionImpl.hpp"

namespace mathter {


	namespace impl {
	template <class T, bool Packed>
	typename std::enable_if<!Quaternion<T, Packed>::SimdAccelerated, Quaternion<T, Packed>>::type Product(const Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
		Quaternion<T, Packed> ret;
		ret.w = lhs.s * rhs.s - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
		ret.x = lhs.s * rhs.x + lhs.x * rhs.s + lhs.y * rhs.z - lhs.z * rhs.y;
		ret.y = lhs.s * rhs.y - lhs.x * rhs.z + lhs.y * rhs.s + lhs.z * rhs.x;
		ret.z = lhs.s * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.s;
		return ret;
	}


	template <class T, bool Packed>
	typename std::enable_if<Quaternion<T, Packed>::SimdAccelerated, Quaternion<T, Packed>>::type Product(const Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
		Quaternion<T, Packed> ret;
		using SimdT = Simd<T, 4>;

		SimdT dabc = lhs.vec.simd;
		SimdT wxyz = rhs.vec.simd;
		SimdT alternate;
		alternate.v[0] = -1;
		alternate.v[1] = 1;
		alternate.v[2] = -1;
		alternate.v[3] = 1;

		// [ 3, 2, 1, 0 ]
		// [ 0, 3, 2, 1 ]
		SimdT t0 = SimdT::template shuffle<0, 0, 0, 0>(dabc);
		SimdT t1 = SimdT::template shuffle<3, 0, 1, 2>(wxyz);

		SimdT t2 = SimdT::template shuffle<1, 1, 1, 1>(dabc);
		SimdT t3 = SimdT::template shuffle<2, 1, 0, 3>(wxyz);

		SimdT t4 = SimdT::template shuffle<2, 2, 2, 2>(dabc);
		SimdT t5 = SimdT::template shuffle<3, 1, 0, 2>(wxyz);

		SimdT m0 = SimdT::mul(t0, t1);
		SimdT m1 = SimdT::mul(t2, t3);
		SimdT m2 = SimdT::mul(t4, t5);

		SimdT t6 = SimdT::template shuffle<3, 3, 3, 3>(dabc);
		SimdT t7 = SimdT::template shuffle<0, 3, 1, 2>(wxyz);

		SimdT m3 = SimdT::mul(t6, t7);

		SimdT e = SimdT::add(m0, SimdT::mul(alternate, m1));
		e = SimdT::template shuffle<1, 3, 0, 2>(e);
		e = SimdT::add(e, SimdT::mul(alternate, m2));
		e = SimdT::template shuffle<2, 0, 1, 3>(e);
		e = SimdT::add(e, SimdT::mul(alternate, m3));
		e = SimdT::template shuffle<3, 1, 0, 2>(e);

		ret.vec.simd = e;
		return ret;
	}
} // namespace impl



template <class T, bool Packed>
Quaternion<T, Packed>& operator+=(Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
	lhs.vec += rhs.vec;
	return lhs;
} // Helpers to write quaternion in paper-style such as (1 + 2_i + 3_j + 4_k). Slow performance, be careful.
template <class T, bool Packed>
Quaternion<T, Packed>& operator-=(Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
	lhs.vec -= rhs.vec;
	return lhs;
}

template <class T, bool Packed>
Quaternion<T, Packed>& operator*=(Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
	lhs = impl::Product(lhs, rhs);
	return lhs;
}

template <class T, bool Packed>
Quaternion<T, Packed>& operator*=(Quaternion<T, Packed>& lhs, T s) {
	lhs.vec *= s;
	return lhs;
}

template <class T, bool Packed>
Quaternion<T, Packed>& operator/=(Quaternion<T, Packed>& lhs, T s) {
	lhs *= T(1) / s;
	return lhs;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator+(const Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
	Quaternion<T, Packed> copy(lhs);
	copy += rhs;
	return copy;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator-(const Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
	Quaternion<T, Packed> copy(lhs);
	copy -= rhs;
	return copy;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator*(const Quaternion<T, Packed>& lhs, const Quaternion<T, Packed>& rhs) {
	Quaternion<T, Packed> copy(lhs);
	copy *= rhs;
	return copy;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator*(const Quaternion<T, Packed>& lhs, T s) {
	Quaternion<T, Packed> copy(lhs);
	copy *= s;
	return copy;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator/(const Quaternion<T, Packed>& lhs, T s) {
	Quaternion<T, Packed> copy(lhs);
	copy /= s;
	return copy;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator+(const Quaternion<T, Packed>& arg) {
	return arg;
}

template <class T, bool Packed>
Quaternion<T, Packed> operator-(const Quaternion<T, Packed>& arg) {
	return Quaternion(-arg.vec);
}


/// <summary> Multiplies all coefficients of the quaternion by <paramref name="s"/>. </summary>
template <class T, bool Packed, class U, class = typename std::enable_if<!std::is_same<U, Quaternion<T, Packed>>::value>::type>
Quaternion<T, Packed> operator*(U s, const Quaternion<T, Packed>& rhs) {
	return rhs * s;
}
/// <summary> Divides all coefficients of the quaternion by <paramref name="s"/>. </summary>
template <class T, bool Packed, class U, class = typename std::enable_if<!std::is_same<U, Quaternion<T, Packed>>::value>::type>
Quaternion<T, Packed> operator/(U s, const Quaternion<T, Packed>& rhs) {
	return rhs / s;
}

/// <summary> Adds a real to the real part of the quaternion. </summary>
template <class T, bool Packed, class U, class = typename std::enable_if<!traits::IsQuaternion<U>::value>::type>
Quaternion<T, Packed> operator+(const U& lhs, const Quaternion<T, Packed>& rhs) {
	return Quaternion<T, Packed>(rhs.w + lhs, rhs.x, rhs.y, rhs.z);
}



} // namespace mathter