#pragma once


// Specialization for floats.
#include "../IoStream.hpp"
#include "../Matrix.hpp"
#include "../Vector.hpp"

#include <type_traits>

namespace mathter {


template <class T>
bool AlmostEqual(T d1, T d2, std::true_type) {
	if (std::abs(d1) < 1e-38 && std::abs(d2) < 1e-38) {
		return true;
	}
	if ((d1 == 0 && d2 < 1e-4) || (d2 == 0 && d1 < 1e-4)) {
		return true;
	}
	T scaler = pow(T(10), floor(std::log10(std::abs(d1))));
	d1 /= scaler;
	d2 /= scaler;
	d1 *= T(1000.0);
	d2 *= T(1000.0);
	return round(d1) == round(d2);
}

// Specialization for int, complex and custom types: simple equality.
template <class T>
bool AlmostEqual(T d1, T d2, std::false_type) {
	return d1 == d2;
}

// Check equivalence with tolerance.
template <class T, class = std::enable_if_t<traits::NotVector<T>::value && traits::NotMatrix<T>::value && traits::NotQuaternion<T>::value>>
bool AlmostEqual(T d1, T d2) {
	return AlmostEqual(d1, d2, std::integral_constant<bool, std::is_floating_point<T>::value>());
}

template <class T, int Dim, bool Packed1, bool Packed2>
bool AlmostEqual(const Vector<T, Dim, Packed1>& lhs, const Vector<T, Dim, Packed2>& rhs) {
	bool eq = true;
	for (auto i : Range(Dim)) {
		eq = eq && AlmostEqual(lhs[i], rhs[i]);
	}
	return eq;
}

template <class T, bool Packed1, bool Packed2>
bool AlmostEqual(const Quaternion<T, Packed1>& lhs, const Quaternion<T, Packed2>& rhs) {
	bool eq = true;
	for (auto i : Range(4)) {
		eq = eq && AlmostEqual(lhs.vec[i], rhs.vec[i]);
	}
	return eq;
}

template <class T,
		  int Rows,
		  int Columns,
		  eMatrixOrder Order1,
		  eMatrixLayout Layout1,
		  bool Packed1,
		  eMatrixOrder Order2,
		  eMatrixLayout Layout2,
		  bool Packed2>
bool AlmostEqual(const Matrix<T, Rows, Columns, Order1, Layout1, Packed1>& lhs, const Matrix<T, Rows, Columns, Order2, Layout2, Packed2>& rhs) {
	bool eq = true;
	for (auto i : Range(Rows)) {
		for (auto j : Range(Columns)) {
			eq = eq && AlmostEqual(lhs(i, j), rhs(i, j));
		}
	}
	return eq;
}


// Floating point comparison helper class, works like Catch2 units testing framework's float Approx.
template <class LinalgClass>
struct ApproxHelper {
	ApproxHelper() {}
	explicit ApproxHelper(LinalgClass object) {
		this->object = object;
	}
	LinalgClass object;
};


template <class LinalgClass1, class LinalgClass2>
bool operator==(const ApproxHelper<LinalgClass1>& lhs, const LinalgClass2& rhs) {
	return AlmostEqual(lhs.object, rhs);
}

template <class LinalgClass1, class LinalgClass2>
bool operator==(const LinalgClass1& lhs, const ApproxHelper<LinalgClass2>& rhs) {
	return AlmostEqual(rhs.object, lhs);
}

template <class LinalgClass1, class LinalgClass2>
bool operator==(const ApproxHelper<LinalgClass1>& lhs, const ApproxHelper<LinalgClass2>& rhs) {
	return AlmostEqual(lhs.object, rhs.object);
}

template <class LinalgClass>
std::ostream& operator<<(std::ostream& os, const ApproxHelper<LinalgClass>& arg) {
	os << arg.object;
	return os;
}

template <class LinalgClass>
ApproxHelper<LinalgClass> ApproxVec(const LinalgClass& arg) {
	return ApproxHelper<LinalgClass>{ arg };
}



} // namespace mathter