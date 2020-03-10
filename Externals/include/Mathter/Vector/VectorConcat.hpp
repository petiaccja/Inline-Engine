#pragma once

#include "VectorImpl.hpp"

namespace mathter {


/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T, int Dim, bool Packed, class U>
mathter::Vector<T, Dim + 1, Packed> operator|(const mathter::Vector<T, Dim, Packed>& lhs, U rhs) {
	mathter::Vector<T, Dim + 1, Packed> ret;
	for (int i = 0; i < Dim; ++i) {
		ret(i) = lhs(i);
	}
	ret(Dim) = rhs;
	return ret;
}

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int Dim1, class T2, int Dim2, bool Packed>
mathter::Vector<T1, Dim1 + Dim2, Packed> operator|(const mathter::Vector<T1, Dim1, Packed>& lhs, const mathter::Vector<T2, Dim2, Packed>& rhs) {
	mathter::Vector<T1, Dim1 + Dim2, Packed> ret;
	for (int i = 0; i < Dim1; ++i) {
		ret(i) = lhs(i);
	}
	for (int i = 0; i < Dim2; ++i) {
		ret(Dim1 + i) = rhs(i);
	}
	return ret;
}

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T, int Dim, bool Packed, class U>
mathter::Vector<T, Dim + 1, Packed> operator|(U lhs, const mathter::Vector<T, Dim, Packed>& rhs) {
	mathter::Vector<T, Dim + 1, Packed> ret;
	ret(0) = lhs;
	for (int i = 0; i < Dim; ++i) {
		ret(i + 1) = rhs(i);
	}
	return ret;
}

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class T2, int... Indices2>
Vector<T1, sizeof...(Indices2) + sizeof...(Indices2), false> operator|(const Swizzle<T1, Indices1...>& lhs, const Swizzle<T2, Indices2...>& rhs) {
	return Vector<T1, sizeof...(Indices1), false>(lhs) | Vector<T1, sizeof...(Indices2), false>(rhs);
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class T2, int Dim, bool Packed>
Vector<T1, sizeof...(Indices1) + Dim, Packed> operator|(const Swizzle<T1, Indices1...>& lhs, const Vector<T2, Dim, Packed>& rhs) {
	return Vector<T1, sizeof...(Indices1), Packed>(lhs) | rhs;
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class T2, int Dim, bool Packed>
Vector<T1, sizeof...(Indices1) + Dim, Packed> operator|(const Vector<T2, Dim, Packed>& lhs, const Swizzle<T1, Indices1...>& rhs) {
	return lhs | Vector<T1, sizeof...(Indices1), false>(rhs);
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class U>
Vector<T1, sizeof...(Indices1) + 1, false> operator|(const Swizzle<T1, Indices1...>& lhs, U rhs) {
	return Vector<T1, sizeof...(Indices1), false>(lhs) | rhs;
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class U>
Vector<T1, sizeof...(Indices1) + 1, false> operator|(U lhs, const Swizzle<T1, Indices1...>& rhs) {
	return lhs | Vector<T1, sizeof...(Indices1), false>(rhs);
}


} // namespace mathter