//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once


#include "Vector.hpp"
#include <limits>
#include "Utility.hpp"


namespace mathter {


/// <summary> Allows you to do quaternion math and represent rotation in a compact way. </summary>
/// <typeparam name="T"> The scalar type of w, x, y and z. Use a builtin or custom floating or fixed point type. </typeparam>
/// <typeparam name="Packed"> If true, tightly packs quaternion members and disables padding due to overalignment in arrays. 
///		Disables SIMD optimization. </typeparam>
/// <remarks>
/// These are plain mathematical quaternions, so expect the operations to work as mathematically defined.
/// There are helper functions to represent rotation with quaternions.
/// </remarks>
template <class T, bool Packed = false>
class Quaternion {
	static constexpr bool SimdAccelerated = impl::HasSimd<Vector<T, 4, Packed>>::value;
public:
	union {
		struct { T s, i, j, k; };
		struct { T w, x, y, z; };
		Vector<T, 4, Packed> vec;
	};

	//-----------------------------------------------
	// Constructors
	//-----------------------------------------------
	/// <summary> Does NOT zero-initialize values. </summary>
	Quaternion() {}

	Quaternion(const Quaternion& rhs) : vec(rhs.vec) {}

	/// <summary> Set values directly. </summary>
	Quaternion(T scalar, T x, T y, T z) : w(scalar), x(x), y(y), z(z) {}

	/// <summary> Sets the scalar part (w) and the vector part (xyz). This is not <see cref="AxisAngle"/> rotation. </summary>
	Quaternion(T scalar, const Vector<T, 3, true>& vector) : w(scalar), x(vector.x), y(vector.y), z(vector.z) {}

	/// <summary> Sets the scalar part (w) and the vector part (xyz). This is not <see cref="AxisAngle"/> rotation. </summary>
	Quaternion(T scalar, const Vector<T, 3, false>& vector) : w(scalar), x(vector.x), y(vector.y), z(vector.z) {}

	/// <summary> Sets the scalar part to zero, and the vector part to given argument. </summary>
	explicit Quaternion(const Vector<T, 3, true>& vector) : Quaternion(0, vector) {}

	template <class U, bool P>
	Quaternion(const Quaternion<U, P>& rhs) : vec(rhs.vec) {}

	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). </remarks>
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	explicit Quaternion(const Matrix<U, 3, 3, Order, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
	}
	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). Translation part is ignored. </remarks>
	template <class U, eMatrixLayout Layout, bool PackedA>
	explicit Quaternion(const Matrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
	}
	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). Translation part is ignored. </remarks>
	template <class U, eMatrixLayout Layout, bool PackedA>
	explicit Quaternion(const Matrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
	}
	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). Translation part is ignored. </remarks>
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	explicit Quaternion(const Matrix<U, 4, 4, Order, Layout, PackedA>& rhs) {
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

	/// <summary> Convert from quaternion with different base type and packing. </summary>
	template <class U, bool P>
	Quaternion& operator=(const Quaternion<U, P>& rhs) {
		vec = rhs.vec;
		return *this;
	}


	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). </remarks>
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	Quaternion& operator=(const Matrix<U, 3, 3, Order, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
		return *this;
	}
	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). Translation part is ignored. </remarks>
	template <class U, eMatrixLayout Layout, bool PackedA>
	Quaternion& operator=(const Matrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
		return *this;
	}
	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). Translation part is ignored. </remarks>
	template <class U, eMatrixLayout Layout, bool PackedA>
	Quaternion& operator=(const Matrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
		return *this;
	}
	/// <summary> Convert a rotation matrix to equivalent quaternion. </summary>
	/// <remarks> Matrix must be in SO(3). Translation part is ignored. </remarks>
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	Quaternion& operator=(const Matrix<U, 4, 4, Order, Layout, PackedA>& rhs) {
		FromMatrix(rhs);
		return *this;
	}


	//-----------------------------------------------
	// Alternate constructions
	//-----------------------------------------------

	/// <summary> Creates a quaternion that performs a rotation of <paramref name="angle"/> radians around <paramref name="axis"/> </summary>
	template <bool VPacked>
	static Quaternion AxisAngle(const Vector<T, 3, VPacked>& axis, T angle) {
		angle *= T(0.5);
		return Quaternion(cos(angle), axis * sin(angle));
	}

	/// <summary> Sets the quaternion's elements to represent a rotation of <paramref name="angle"/> radians around <paramref name="axis"/> </summary>
	template <bool VPacked>
	Quaternion& SetAxisAngle(const Vector<T, 3, VPacked>& axis, T angle) {
		return *this = AxisAngle(axis, angle);
	}

	/// <summary> Creates an identity quaternion that causes no rotation. </summary>
	static Quaternion Identity() {
		return Quaternion(1, 0, 0, 0);
	}

	/// <summary> Sets this to an identity quaternion that causes no rotation. </summary>
	Quaternion& SetIdentity() {
		return *this = Identity();
	}


	//-----------------------------------------------
	// Arithmetic
	//-----------------------------------------------

	/// <summary> Mathematical quaternion addition. </summary>
	Quaternion& operator+=(const Quaternion& rhs) {
		vec += rhs.vec;
		return *this;
	}

	/// <summary> Mathematical quaternion subtraction. </summary>
	Quaternion& operator-=(const Quaternion& rhs) {
		vec -= rhs.vec;
		return *this;
	}

	/// <summary> Mathematical quaternion product according to distributive law. </summary>
	Quaternion& operator*=(const Quaternion& rhs) {
		*this = Product<SimdAccelerated>(*this, rhs);
		return *this;
	}

	/// <summary> Multiply all coefficients of the quaternion by <paramref name="s"/>. </summary>
	Quaternion& operator*=(T s) {
		vec *= s;
		return *this;
	}
	/// <summary> Divide all coefficients of the quaternion by <paramref name="s"/>. </summary>
	Quaternion& operator/=(T s) {
		*this *= T(1) / s;
		return *this;
	}

	/// <summary> Mathematical quaternion addition. </summary>
	Quaternion operator+(const Quaternion& rhs) const {
		return Quaternion(*this) += rhs;
	}
	/// <summary> Mathematical quaternion subtraction. </summary>
	Quaternion operator-(const Quaternion& rhs) const {
		return Quaternion(*this) -= rhs;
	}
	/// <summary> Mathematical quaternion product according to distributive law. </summary>
	Quaternion operator*(const Quaternion& rhs) const {
		return Quaternion(*this) *= rhs;
	}
	/// <summary> Multiply all coefficients of the quaternion by <paramref name="s"/>. </summary>
	Quaternion operator*(T s) const {
		return Quaternion(*this) *= s;
	}
	/// <summary> Divide all coefficients of the quaternion by <paramref name="s"/>. </summary>
	Quaternion operator/(T s) const {
		return Quaternion(*this) /= s;
	}

	/// <summary> Negates all elements of the quaternion. </summary> 
	/// <remarks> Funnily, it's going to represent the same rotation. </summary>
	Quaternion operator-() const {
		return Quaternion(-vec);
	}

	//-----------------------------------------------
	// Comparison
	//-----------------------------------------------

	/// <summary> Check exact equality of coefficients. </summary>
	bool operator==(const Quaternion& rhs) const {
		return vec == rhs.vec;
	}
	/// <summary> Check exact unequality of coefficients. </summary>
	bool operator!=(const Quaternion& rhs) const {
		return !(*this == rhs);
	}
	/// <summary> Check equality with some tolerance for floats. </summary>
	bool AlmostEqual(const Quaternion& rhs) const {
		return vec.AlmostEqual(rhs.vec);
	}
	auto Approx() const {
		return mathter::ApproxHelper<Quaternion>(*this);
	}

	//-----------------------------------------------
	// Mathematically named functions
	//-----------------------------------------------

	T Abs() const {
		return vec.Length();
	}

	Quaternion Conjugate() const {
		return Inverse();
	}

	static Quaternion Exp(const Quaternion& q) {
		auto a = q.ScalarPart();
		auto v = q.VectorPart();
		T mag = v.Length();
		T es = exp(a);

		Quaternion ret = {std::cos(mag), v*(std::sin(mag)/mag)};
		ret *= es;

		return ret;
	}

	static Quaternion Log(const Quaternion& q) {
		auto magq = q.Length();
		auto vn = q.VectorPart().Normalized();
		
		Quaternion ret = { std::log(magq), vn*acos(q.s / magq) };
		return ret;
	}

	static Quaternion Pow(const Quaternion& q, T a) {
		return Exp(a*Log(q));
	}

	//-----------------------------------------------
	// Functions
	//-----------------------------------------------

	/// <summary> Returns the absolute value of the quaternion. </summary>
	/// <remarks> Just like complex numbers, it's the length of the vector formed by the coefficients. </remarks>
	T Length() const {
		return Abs();
	}
	/// <summary> Returns the square of the absolute value. </summary>
	/// <remarks> Just like complex numbers, it's the square of the length of the vector formed by the coefficients.
	///			This is much faster than <see cref="Length">. </remarks>
	T LengthSquared() const {
		return vec.LengthSquared();
	}

	/// <summary> Returns the unit quaternion of the same direction. Does not change this object. </summary>
	Quaternion Normalized() const {
		return Quaternion(vec.Normalized());
	}
	/// <summary> Makes this a unit quaternion. </summary>
	Quaternion& Normalize() {
		vec.Normalize();
		return *this;
	}

	/// <summary> Reverses the direction of the rotation. </summary>
	Quaternion& Invert() {
		vec *= Vector<T, 4, Packed>{ T(1), T(-1), T(-1), T(-1) };
		return *this;
	}
	/// <summary> Returns the quaternion of opposite rotation. </summary>
	Quaternion Inverse() const {
		return Quaternion(*this).Invert();
	}

	/// <summary> Check if the quaternion is a unit quaternion, with some tolerance for floats. </summary>
	bool IsNormalized() const {
		T n = LengthSquared();
		return T(0.9999) <= n && n <= T(1.0001);
	}

	/// <summary> Returns the scalar part (w) of (w + xi + yj + zk). </summary>
	const T ScalarPart() const {
		return s;
	}
	/// <summary> Returns the vector part (x, y, z) of (w + xi + yj + zk). </summary>
	const Vector<T, 3, Packed> VectorPart() const {
		return { x, y, z };
	}

	/// <summary> Returns the angle of the rotation represented by quaternion. </summary>
	/// <remarks> Only valid for unit quaternions. </remarks>
	const T Angle() const {
		return impl::sign_nonzero(s) * 2 * std::acos(Clamp(abs(s)/Length(), T(-1), T(1)));
	}
	/// <summary> Returns the axis of rotation represented by quaternion. </summary>
	/// <remarks> Only valid for unit quaternions. Returns (1,0,0) for near 180 degree rotations. </remarks>
	const Vector<T, 3, Packed> Axis() const {
		auto direction = VectorPart();
		return direction.SafeNormalized();
	}

	//-----------------------------------------------
	// Matrix conversions
	//-----------------------------------------------

	/// <summary> Creates a rotation matrix equivalent to the quaternion. </summary>
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	explicit operator Matrix<U, 3, 3, Order, Layout, PackedA>() const {
		return ToMatrix<U, 3, 3, Order, Layout, PackedA>();
	}
	/// <summary> Creates a rotation matrix equivalent to the quaternion. </summary>
	template <class U, eMatrixLayout Layout, bool PackedA>
	explicit operator Matrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, PackedA>() const {
		return ToMatrix<U, 3, 4, eMatrixOrder::PRECEDE_VECTOR, Layout, PackedA>();
	}
	/// <summary> Creates a rotation matrix equivalent to the quaternion. </summary>
	template <class U, eMatrixLayout Layout, bool PackedA>
	explicit operator Matrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, PackedA>() const {
		return ToMatrix<U, 4, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, PackedA>();
	}
	/// <summary> Creates a rotation matrix equivalent to the quaternion. </summary>
	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	explicit operator Matrix<U, 4, 4, Order, Layout, PackedA>() const {
		return ToMatrix<U, 4, 4, Order, Layout, PackedA>();
	}

	//-----------------------------------------------
	// Truncate to vector
	//-----------------------------------------------

	/// <summary> Truncates the quaternion to the vector part (x, y, z). </summary>
	template <class U, bool PackedA>
	explicit operator Vector<U, 3, PackedA>() const {
		return { x, y, z };
	}


	//-----------------------------------------------
	// Apply to vector
	//-----------------------------------------------

	/// <summary> Rotates (and scales) vector by quaternion. </summary>
	template <bool PackedA>
	Vector<T, 3, PackedA> operator*(const Vector<T, 3, PackedA>& vec) const {
		// sandwich product
		return Vector<T, 3, PackedA>((*this)*Quaternion(vec)*Inverse());
	}

	/// <summary> Rotates (and scales) vector by quaternion. </summary>
	template <bool PackedA>
	friend Vector<T, 3, PackedA> operator*(const Vector<T, 3, PackedA>& vec, const Quaternion& q) {
		// sandwich product
		return Vector<T, 3, PackedA>(q*Quaternion(vec)*q.Inverse());
	}

	/// <summary> Rotates (and scales) vector by quaternion. </summary>
	template <bool PackedA>
	friend Vector<T, 3, PackedA>& operator*=(Vector<T, 3, PackedA>& vec, const Quaternion& q) {
		// sandwich product
		return vec = Vector<T, 3, PackedA>(q*Quaternion(vec)*q.Inverse());
	}

	/// <summary> Rotates (and scales) vector by quaternion. </summary>
	template <bool PackedA>
	Vector<T, 3, PackedA> operator()(const Vector<T, 3, PackedA>& vec) const {
		// sandwich product
		return (Vector<T, 3, PackedA>)((*this)*Quaternion(vec)*Inverse());
	}

protected:
	//-----------------------------------------------
	// Matrix conversion helpers
	//-----------------------------------------------
	template <class U, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	Matrix<U, Rows, Columns, Order, Layout, PackedA> ToMatrix() const {
		assert(IsNormalized());
		Matrix<U, Rows, Columns, Order, Layout, PackedA> mat;
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

	template <class U, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool PackedA>
	void FromMatrix(const Matrix<U, Rows, Columns, Order, Layout, PackedA>& mat) {
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
	inline static typename std::enable_if<!SimdAccelerated, Quaternion>::type Product(const Quaternion& lhs, const Quaternion& rhs) {
		Quaternion ret;
		ret.w = lhs.s*rhs.s - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z;
		ret.x = lhs.s*rhs.x + lhs.x*rhs.s + lhs.y*rhs.z - lhs.z*rhs.y;
		ret.y = lhs.s*rhs.y - lhs.x*rhs.z + lhs.y*rhs.s + lhs.z*rhs.x;
		ret.z = lhs.s*rhs.z + lhs.x*rhs.y - lhs.y*rhs.x + lhs.z*rhs.s;
		return ret;
	}


	template <bool SimdAccelerated>
	inline static typename std::enable_if<SimdAccelerated, Quaternion>::type Product(const Quaternion& lhs, const Quaternion& rhs) {
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
		SimdT t0 = SimdT::template shuffle<0,0,0,0>(dabc);
		SimdT t1 = SimdT::template shuffle<3,0,1,2>(wxyz);

		SimdT t2 = SimdT::template shuffle<1,1,1,1>(dabc);
		SimdT t3 = SimdT::template shuffle<2,1,0,3>(wxyz);

		SimdT t4 = SimdT::template shuffle<2,2,2,2>(dabc);
		SimdT t5 = SimdT::template shuffle<3,1,0,2>(wxyz);

		SimdT m0 = SimdT::mul(t0, t1);
		SimdT m1 = SimdT::mul(t2, t3);
		SimdT m2 = SimdT::mul(t4, t5);

		SimdT t6 = SimdT::template shuffle<3,3,3,3>(dabc);
		SimdT t7 = SimdT::template shuffle<0,3,1,2>(wxyz);

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
};




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
template <class T, bool Packed, class U, class = typename std::enable_if<!impl::IsQuaternion<U>::value>::type>
Quaternion<T, Packed> operator+(const U& lhs, const Quaternion<T, Packed>& rhs) {
	return Quaternion<T, Packed>(rhs.w + lhs, rhs.x, rhs.y, rhs.z);
}



// Helpers to write quaternion in paper-style such as (1 + 2_i + 3_j + 4_k). Slow performance, be careful.
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

} // namespace quat_literals


template <class T, bool Packed>
std::ostream& operator<<(std::ostream& os, const Quaternion<T, Packed>& q) {
	os << "["
		<< q.Angle() * T(180.0) / T(3.1415926535897932384626)
		<< " deg @ "
		<< q.Axis()
		<< "]";
	return os;
}


} // namespace mathter