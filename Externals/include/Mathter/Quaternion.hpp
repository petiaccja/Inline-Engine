//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once


#include "Vector.hpp"
#include <limits>
#include "Utility.hpp"


namespace mathter {


template <class T, bool Packed = false>
class Quaternion {
	static constexpr bool SimdAccelerated = has_simd<Vector<T, 4, Packed>>::value;
public:
	union {
		struct { T s, i, j, k; };
		struct { T w, x, y, z; };
		Vector<T, 4, Packed> vec;
	};

	//-----------------------------------------------
	// Constructors
	//-----------------------------------------------
	Quaternion() {}
	Quaternion(const Quaternion& rhs) : vec(rhs.vec) {}
	Quaternion(T scalar, T x, T y, T z) : s(scalar), x(x), y(y), z(z) {}
	Quaternion(T scalar, const Vector<T, 3, true>& vector) : s(scalar), x(vector.x), y(vector.y), z(vector.z) {}
	Quaternion(T scalar, const Vector<T, 3, false>& vector) : s(scalar), x(vector.x), y(vector.y), z(vector.z) {}
	explicit Quaternion(const Vector<T, 3, true>& vector) : Quaternion(0, vector) {}

	template <class U, bool P>
	Quaternion(const Quaternion<U, P>& rhs) : vec(rhs.vec) {}

	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	explicit Quaternion(const Matrix<U, 3, 3, Order, Layout, Packed>& rhs) {
		FromMatrix(rhs);
	}
	template <class U, eMatrixLayout Layout, bool Packed>
	explicit Quaternion(const Matrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, Packed>& rhs) {
		FromMatrix(rhs);
	}
	template <class U, eMatrixLayout Layout, bool Packed>
	explicit Quaternion(const Matrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, Packed>& rhs) {
		FromMatrix(rhs);
	}
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	explicit Quaternion(const Matrix<U, 4, 4, Order, Layout, Packed>& rhs) {
		FromMatrix(rhs);
	}
protected:
	explicit Quaternion(const Vector<T, 4, false>& vec) : vec(vec) {}
public:
	//-----------------------------------------------
	// Assignment
	//-----------------------------------------------
	Quaternion& operator=(const Quaternion& rhs) {
		vec = rhs.vec;
		return *this;
	}

	template <class U, bool P>
	Quaternion& operator=(const Quaternion<U, P>& rhs) {
		vec = rhs.vec;
		return *this;
	}


	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	Quaternion& operator=(const Matrix<U, 3, 3, Order, Layout, Packed>& rhs) {
		FromMatrix(rhs);
		return *this;
	}
	template <class U, eMatrixLayout Layout, bool Packed>
	Quaternion& operator=(const Matrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, Packed>& rhs) {
		FromMatrix(rhs);
		return *this;
	}
	template <class U, eMatrixLayout Layout, bool Packed>
	Quaternion& operator=(const Matrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, Packed>& rhs) {
		FromMatrix(rhs);
		return *this;
	}
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	Quaternion& operator=(const Matrix<U, 4, 4, Order, Layout, Packed>& rhs) {
		FromMatrix(rhs);
		return *this;
	}


	//-----------------------------------------------
	// Alternate constructions
	//-----------------------------------------------

	template <bool VPacked>
	static Quaternion AxisAngle(const Vector<T, 3, VPacked>& axis, T angle) {
		angle *= T(0.5);
		return Quaternion(cos(angle), axis * sin(angle));
	}


	//-----------------------------------------------
	// Arithmetic
	//-----------------------------------------------
	Quaternion& operator+=(const Quaternion& rhs) {
		vec += rhs.vec;
		return *this;
	}

	Quaternion& operator-=(const Quaternion& rhs) {
		vec -= rhs.vec;
		return *this;
	}

	Quaternion& operator*=(const Quaternion& rhs) {
		*this = Product<SimdAccelerated>(*this, rhs);
		return *this;
	}

	Quaternion& operator*=(T s) {
		vec *= s;
		return *this;
	}
	Quaternion& operator/=(T s) {
		*this *= T(1) / s;
		return *this;
	}

	Quaternion operator+(const Quaternion& rhs) const {
		return Quaternion(*this) += rhs;
	}
	Quaternion operator-(const Quaternion& rhs) const {
		return Quaternion(*this) -= rhs;
	}
	Quaternion operator*(const Quaternion& rhs) const {
		return Quaternion(*this) *= rhs;
	}
	Quaternion operator*(T s) const {
		return Quaternion(*this) *= s;
	}
	Quaternion operator/(T s) const {
		return Quaternion(*this) /= s;
	}

	Quaternion operator-() const {
		return Quaternion(-vec);
	}

	//-----------------------------------------------
	// Comparison
	//-----------------------------------------------
	bool operator==(const Quaternion& rhs) const {
		return vec == rhs.vec;
	}
	bool operator!=(const Quaternion& rhs) const {
		return !(*this == rhs);
	}
	bool AlmostEqual(const Quaternion& rhs) const {
		return vec.AlmostEqual(rhs.vec);
	}


	//-----------------------------------------------
	// Functions
	//-----------------------------------------------

	T Length() const {
		return vec.Length();
	}
	T LengthSquared() const {
		return vec.LengthSquared();
	}

	Quaternion Normalized() const {
		return Quaternion(vec.Normalized());
	}
	Quaternion& Normalize() {
		vec.Normalize();
		return *this;
	}

	Quaternion& Invert() {
		vec *= Vector<T, 4, Packed>{ T(1), T(-1), T(-1), T(-1) };
		return *this;
	}
	Quaternion Inverse() const {
		return Quaternion(*this).Invert();
	}

	bool IsNormalized() const {
		T n = LengthSquared();
		return T(0.9999) <= n && n <= T(1.0001);
	}

	const T ScalarPart() const {
		return s;
	}
	const Vector<T, 3, Packed> VectorPart() const {
		return { x, y, z };
	}

	const T Angle() const {
		assert(IsNormalized());
		return 2 * std::acos(s);
	}
	const Vector<T, 3, Packed> Axis() const {
		assert(IsNormalized());
		auto direction = VectorPart();
		T sgnx = direction.x >= T(0.0) ? T(1.0) : T(-1.0);
		static constexpr T epsilon = T(1) / impl::ConstexprExp10<T>(impl::ConstexprAbs(std::numeric_limits<T>::min_exponent10) / 2);
		direction.x += sgnx * epsilon; // Compensation is needed so that degenerated nullvector axis can be normalized.
		return direction.Normalized();
	}

	//-----------------------------------------------
	// Matrix conversions
	//-----------------------------------------------
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	explicit operator Matrix<U, 3, 3, Order, Layout, Packed>() const {
		return ToMatrix<U, 3, 3, Order, Layout, Packed>();
	}
	template <class U, eMatrixLayout Layout, bool Packed>
	explicit operator Matrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, Packed>() const {
		return ToMatrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, Packed>();
	}
	template <class U, eMatrixLayout Layout, bool Packed>
	explicit operator Matrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, Packed>() const {
		return ToMatrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, Packed>();
	}
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	explicit operator Matrix<U, 4, 4, Order, Layout, Packed>() const {
		return ToMatrix<U, 4, 4, Order, Layout, Packed>();
	}

	//-----------------------------------------------
	// Truncate to vector
	//-----------------------------------------------
	template <class U, bool Packed>
	explicit operator Vector<U, 3, Packed>() const {
		return { x, y, z };
	}


	//-----------------------------------------------
	// Apply to vector
	//-----------------------------------------------
	template <bool Packed>
	Vector<T, 3, Packed> operator*(const Vector<T, 3, Packed>& vec) const {
		// sandwich product
		return Vector<T, 3, Packed>((*this)*Quaternion(vec)*Inverse());
	}

	template <bool Packed>
	friend Vector<T, 3, Packed> operator*(const Vector<T, 3, Packed>& vec, const Quaternion& q) {
		// sandwich product
		return Vector<T, 3, Packed>(q*Quaternion(vec)*q.Inverse());
	}


	template <bool Packed>
	friend Vector<T, 3, Packed>& operator*=(Vector<T, 3, Packed>& vec, const Quaternion& q) {
		// sandwich product
		return vec = Vector<T, 3, Packed>(q*Quaternion(vec)*q.Inverse());
	}

	template <bool Packed>
	Vector<T, 3, Packed> operator()(const Vector<T, 3, Packed>& vec) const {
		// sandwich product
		return (Vector<T, 3, Packed>)(q*Quaternion(vec)*q.Inverse());
	}

protected:
	//-----------------------------------------------
	// Matrix conversion helpers
	//-----------------------------------------------
	template <class U, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	Matrix<U, Rows, Columns, Order, Layout, Packed> ToMatrix() const {
		assert(IsNormalized());
		Matrix<U, Rows, Columns, Order, Layout, Packed> mat;
		auto elem = [&mat](int i, int j) -> U& {
			return Order == eMatrixOrder::PRECEDE_VECTOR ? mat(i, j) : mat(j, i);
		};
		elem(0, 0) = 1 - 2 * (j*j + k*k);	elem(0, 1) = 2 * (i*j - k*s);		elem(0, 2) = 2 * (i*k + j*s);
		elem(1, 0) = 2 * (i*j + k*s);		elem(1, 1) = 1 - 2 * (i*i + k*k);	elem(1, 2) = 2 * (j*k - i*s);
		elem(2, 0) = 2 * (i*k - j*s);		elem(2, 1) = 2 * (j*k + i*s);		elem(2, 2) = 1 - 2 * (i*i + j*j);

		// Rest
		for (int j = 0; j < mat.Width(); ++j) {
			for (int i = (j < 3 ? 3 : 0); i < mat.Height(); ++i) {
				mat(i, j) = T(j == i);
			}
		}

		return mat;
	}

	template <class U, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
	void FromMatrix(const Matrix<U, Rows, Columns, Order, Layout, Packed>& mat) {
		assert(mat.IsRotationMatrix3D());
		auto elem = [&mat](int i, int j) -> U {
			return Order == eMatrixOrder::PRECEDE_VECTOR ? mat(i, j) : mat(j, i);
		};
		w = std::sqrt(1 + elem(0, 0) + elem(1, 1) + elem(2, 2)) * T(0.5);
		T div = T(1) / (T(4) * w);
		x = (elem(2, 1) - elem(1, 2)) * div;
		y = (elem(0, 2) - elem(2, 0)) * div;
		z = (elem(1, 0) - elem(0, 1)) * div;
	}

	template <bool SimdAccelerated>
	inline static Quaternion Product(const Quaternion& lhs, const Quaternion& rhs) {
		Quaternion ret;
		ret.w = lhs.s*rhs.s - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z;
		ret.x = lhs.s*rhs.x + lhs.x*rhs.s + lhs.y*rhs.z - lhs.z*rhs.y;
		ret.y = lhs.s*rhs.y - lhs.x*rhs.z + lhs.y*rhs.s + lhs.z*rhs.x;
		ret.z = lhs.s*rhs.z + lhs.x*rhs.y - lhs.y*rhs.x + lhs.z*rhs.s;
		return ret;
	}

	template <>
	inline static Quaternion Product<true>(const Quaternion& lhs, const Quaternion& rhs) {
		Quaternion ret;
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
		SimdT t0 = SimdT::shuffle<0,0,0,0>(dabc);
		SimdT t1 = SimdT::shuffle<3,0,1,2>(wxyz);

		SimdT t2 = SimdT::shuffle<1,1,1,1>(dabc);
		SimdT t3 = SimdT::shuffle<2,1,0,3>(wxyz);

		SimdT t4 = SimdT::shuffle<2,2,2,2>(dabc);
		SimdT t5 = SimdT::shuffle<3,1,0,2>(wxyz);

		SimdT m0 = SimdT::mul(t0, t1);
		SimdT m1 = SimdT::mul(t2, t3);
		SimdT m2 = SimdT::mul(t4, t5);

		SimdT t6 = SimdT::shuffle<3,3,3,3>(dabc);
		SimdT t7 = SimdT::shuffle<0,3,1,2>(wxyz);

		SimdT m3 = SimdT::mul(t6, t7);

		SimdT e = SimdT::add(m0, SimdT::mul(alternate, m1));
		e = SimdT::shuffle<1, 3, 0, 2>(e);
		e = SimdT::add(e, SimdT::mul(alternate, m2));
		e = SimdT::shuffle<2, 0, 1, 3>(e);
		e = SimdT::add(e, SimdT::mul(alternate, m3));
		e = SimdT::shuffle<3, 1, 0, 2>(e);

		ret.vec.simd = e;
		return ret;
	}
};



template <class T, bool Packed, class U, class = typename std::enable_if<!std::is_same<U, Quaternion<T, Packed>>::value>::type>
Quaternion<T, Packed> operator*(U s, const Quaternion<T, Packed>& rhs) {
	return rhs * s;
}
template <class T, bool Packed, class U, class = typename std::enable_if<!std::is_same<U, Quaternion<T, Packed>>::value>::type>
Quaternion<T, Packed> operator/(U s, const Quaternion<T, Packed>& rhs) {
	return rhs / s;
}

template <class T, bool Packed, class U, class = typename std::enable_if<!impl::IsQuaternion<U>::value>::type>
Quaternion<T, Packed> operator+(const U& lhs, const Quaternion<T, Packed>& rhs) {
	return Quaternion<T, Packed>(rhs.w + lhs, rhs.x, rhs.y, rhs.z);
}



namespace quat_literals {

inline Quaternion<long double> operator "" _i(unsigned long long int arg) {
	return Quaternion<long double>(0, (long double)arg, 0, 0);
}
inline Quaternion<long double> operator "" _j(unsigned long long int arg) {
	return Quaternion<long double>(0, 0, (long double)arg, 0);
}
inline Quaternion<long double> operator "" _k(unsigned long long int arg) {
	return Quaternion<long double>(0, 0, 0, (long double)arg);
}

inline Quaternion<long double> operator "" _i(long double arg) {
	return Quaternion<long double>(0, arg, 0, 0);
}
inline Quaternion<long double> operator "" _j(long double arg) {
	return Quaternion<long double>(0, 0, arg, 0);
}
inline Quaternion<long double> operator "" _k(long double arg) {
	return Quaternion<long double>(0, 0, 0, arg);
}

};



} // namespace mathter