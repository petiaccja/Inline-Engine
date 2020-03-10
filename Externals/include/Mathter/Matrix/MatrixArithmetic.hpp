#pragma once

#include "MatrixImpl.hpp"

namespace mathter {
	
// Macros for manual matrix multiplication loop unrolling

// Row-major * Row-major
#define MATHTER_MATMUL_EXPAND(...) __VA_ARGS__

#define MATHTER_MATMUL_RR_FACTOR(X, Y) rhs.stripes[X] * lhs(Y, X)

#define MATHTER_MATMUL_RR_STRIPE_1(Y) MATHTER_MATMUL_RR_FACTOR(0, Y)
#define MATHTER_MATMUL_RR_STRIPE_2(Y) MATHTER_MATMUL_RR_STRIPE_1(Y) + MATHTER_MATMUL_RR_FACTOR(1, Y)
#define MATHTER_MATMUL_RR_STRIPE_3(Y) MATHTER_MATMUL_RR_STRIPE_2(Y) + MATHTER_MATMUL_RR_FACTOR(2, Y)
#define MATHTER_MATMUL_RR_STRIPE_4(Y) MATHTER_MATMUL_RR_STRIPE_3(Y) + MATHTER_MATMUL_RR_FACTOR(3, Y)
#define MATHTER_MATMUL_RR_STRIPE(CX, Y)                  \
	MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_RR_STRIPE_##CX) \
	(Y)

#define MATHTER_MATMUL_RR_ARRAY_1(CX) result.stripes[0] = MATHTER_MATMUL_RR_STRIPE(CX, 0);
#define MATHTER_MATMUL_RR_ARRAY_2(CX) \
	MATHTER_MATMUL_RR_ARRAY_1(CX)     \
	result.stripes[1] = MATHTER_MATMUL_RR_STRIPE(CX, 1);
#define MATHTER_MATMUL_RR_ARRAY_3(CX) \
	MATHTER_MATMUL_RR_ARRAY_2(CX)     \
	result.stripes[2] = MATHTER_MATMUL_RR_STRIPE(CX, 2);
#define MATHTER_MATMUL_RR_ARRAY_4(CX) \
	MATHTER_MATMUL_RR_ARRAY_3(CX)     \
	result.stripes[3] = MATHTER_MATMUL_RR_STRIPE(CX, 3);

#define MATHTER_MATMUL_RR_ARRAY(CX, CY)                 \
	MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_RR_ARRAY_##CY) \
	(CX)

#define MATHTER_MATMUL_RR_UNROLL(MATCH, ROWS1) \
	if (Rows1 == ROWS1 && Match == MATCH) {    \
		MATHTER_MATMUL_RR_ARRAY(MATCH, ROWS1)  \
		return result;                         \
	}

// Column-major * Column-major
#define MATHTER_MATMUL_CC_FACTOR(X, Y) lhs.stripes[Y] * rhs(Y, X)

#define MATHTER_MATMUL_CC_STRIPE_1(X) MATHTER_MATMUL_CC_FACTOR(X, 0)
#define MATHTER_MATMUL_CC_STRIPE_2(X) MATHTER_MATMUL_CC_STRIPE_1(X) + MATHTER_MATMUL_CC_FACTOR(X, 1)
#define MATHTER_MATMUL_CC_STRIPE_3(X) MATHTER_MATMUL_CC_STRIPE_2(X) + MATHTER_MATMUL_CC_FACTOR(X, 2)
#define MATHTER_MATMUL_CC_STRIPE_4(X) MATHTER_MATMUL_CC_STRIPE_3(X) + MATHTER_MATMUL_CC_FACTOR(X, 3)
#define MATHTER_MATMUL_CC_STRIPE(CY, X)                  \
	MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_CC_STRIPE_##CY) \
	(X)

#define MATHTER_MATMUL_CC_ARRAY_1(CY) result.stripes[0] = MATHTER_MATMUL_CC_STRIPE(CY, 0);
#define MATHTER_MATMUL_CC_ARRAY_2(CY) \
	MATHTER_MATMUL_CC_ARRAY_1(CY)     \
	result.stripes[1] = MATHTER_MATMUL_CC_STRIPE(CY, 1);
#define MATHTER_MATMUL_CC_ARRAY_3(CY) \
	MATHTER_MATMUL_CC_ARRAY_2(CY)     \
	result.stripes[2] = MATHTER_MATMUL_CC_STRIPE(CY, 2);
#define MATHTER_MATMUL_CC_ARRAY_4(CY) \
	MATHTER_MATMUL_CC_ARRAY_3(CY)     \
	result.stripes[3] = MATHTER_MATMUL_CC_STRIPE(CY, 3);

#define MATHTER_MATMUL_CC_ARRAY(CX, CY)                 \
	MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_CC_ARRAY_##CX) \
	(CY)

#define MATHTER_MATMUL_CC_UNROLL(COLUMNS2, MATCH) \
	if (Columns2 == COLUMNS2 && Match == MATCH) { \
		MATHTER_MATMUL_CC_ARRAY(COLUMNS2, MATCH)  \
		return result;                            \
	}



template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::ROW_MAJOR, Packed>& lhs,
					  const Matrix<U, Match, Columns2, Order2, eMatrixLayout::ROW_MAJOR, Packed>& rhs)
	-> Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed> {
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed> result;

	MATHTER_MATMUL_RR_UNROLL(2, 2);
	MATHTER_MATMUL_RR_UNROLL(2, 3);
	MATHTER_MATMUL_RR_UNROLL(2, 4);

	MATHTER_MATMUL_RR_UNROLL(3, 2);
	MATHTER_MATMUL_RR_UNROLL(3, 3);
	MATHTER_MATMUL_RR_UNROLL(3, 4);

	MATHTER_MATMUL_RR_UNROLL(4, 2);
	MATHTER_MATMUL_RR_UNROLL(4, 3);
	MATHTER_MATMUL_RR_UNROLL(4, 4);

	// general algorithm
	for (int i = 0; i < Rows1; ++i) {
		result.stripes[i] = rhs.stripes[0] * lhs(i, 0);
	}
	for (int i = 0; i < Rows1; ++i) {
		for (int j = 1; j < Match; ++j) {
			result.stripes[i] += rhs.stripes[j] * lhs(i, j);
		}
	}

	return result;
}

template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::ROW_MAJOR, Packed>& lhs,
					  const Matrix<U, Match, Columns2, Order2, eMatrixLayout::COLUMN_MAJOR, Packed>& rhs)
	-> Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed> {
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed> result;

	for (int j = 0; j < Columns2; ++j) {
		for (int i = 0; i < Rows1; ++i) {
			result(i, j) = Dot(lhs.stripes[i], rhs.stripes[j]);
		}
	}

	return result;
}

template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::COLUMN_MAJOR, Packed>& lhs,
					  const Matrix<U, Match, Columns2, Order2, eMatrixLayout::COLUMN_MAJOR, Packed>& rhs)
	-> Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed> {
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed> result;

	MATHTER_MATMUL_CC_UNROLL(2, 2);
	MATHTER_MATMUL_CC_UNROLL(2, 3);
	MATHTER_MATMUL_CC_UNROLL(2, 4);

	MATHTER_MATMUL_CC_UNROLL(3, 2);
	MATHTER_MATMUL_CC_UNROLL(3, 3);
	MATHTER_MATMUL_CC_UNROLL(3, 4);

	MATHTER_MATMUL_CC_UNROLL(4, 2);
	MATHTER_MATMUL_CC_UNROLL(4, 3);
	MATHTER_MATMUL_CC_UNROLL(4, 4);

	// general algorithm
	for (int j = 0; j < Columns2; ++j) {
		result.stripes[j] = lhs.stripes[0] * rhs(0, j);
	}
	for (int i = 1; i < Match; ++i) {
		for (int j = 0; j < Columns2; ++j) {
			result.stripes[j] += lhs.stripes[i] * rhs(i, j);
		}
	}

	return result;
}

template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::COLUMN_MAJOR, Packed>& lhs,
					  const Matrix<U, Match, Columns2, Order2, eMatrixLayout::ROW_MAJOR, Packed>& rhs)
	-> Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed> {
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed> result;

	MATHTER_MATMUL_CC_UNROLL(2, 2);
	MATHTER_MATMUL_CC_UNROLL(2, 3);
	MATHTER_MATMUL_CC_UNROLL(2, 4);

	MATHTER_MATMUL_CC_UNROLL(3, 2);
	MATHTER_MATMUL_CC_UNROLL(3, 3);
	MATHTER_MATMUL_CC_UNROLL(3, 4);

	MATHTER_MATMUL_CC_UNROLL(4, 2);
	MATHTER_MATMUL_CC_UNROLL(4, 3);
	MATHTER_MATMUL_CC_UNROLL(4, 4);

	// general algorithm
	// CC algorithm is completely fine for COL_MAJOR x ROW_MAJOR
	// see that rhs is only indexed per-element, so its layout does not matter
	for (int j = 0; j < Columns2; ++j) {
		result.stripes[j] = lhs.stripes[0] * rhs(0, j);
	}
	for (int i = 1; i < Match; ++i) {
		for (int j = 0; j < Columns2; ++j) {
			result.stripes[j] += lhs.stripes[i] * rhs(i, j);
		}
	}

	return result;
}


template <class T1, class T2, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool PackedA>
inline auto operator*(const Matrix<T1, Rows1, Match, Order1, Layout1, PackedA>& lhs,
					  const Matrix<T2, Match, Columns2, Order2, Layout2, PackedA>& rhs)
	-> Matrix<traits::MatMulElemT<T1, T2>, Rows1, Columns2, Order1, Layout1, PackedA> {
	return mathter::operator*<T1, T2, Rows1, Match, Columns2, Order1, Order2, PackedA, traits::MatMulElemT<T1, T2>>(lhs, rhs);
}


// Assign-multiply
template <class T1, class T2, int Dim, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed>
inline Matrix<T1, Dim, Dim, Order1, Layout1, Packed>& operator*=(Matrix<T1, Dim, Dim, Order1, Layout1, Packed>& lhs, const Matrix<T2, Dim, Dim, Order2, Layout2, Packed>& rhs) {
	lhs = operator*<T1, T2, Dim, Dim, Dim, Order1, Order2, Packed, T1>(lhs, rhs);
	return lhs;
}

#define MATHTER_MATADD_SAME_ARRAY_1(OP) result.stripes[0] = lhs.stripes[0] OP rhs.stripes[0];
#define MATHTER_MATADD_SAME_ARRAY_2(OP) \
	MATHTER_MATADD_SAME_ARRAY_1(OP)     \
	result.stripes[1] = lhs.stripes[1] OP rhs.stripes[1];
#define MATHTER_MATADD_SAME_ARRAY_3(OP) \
	MATHTER_MATADD_SAME_ARRAY_2(OP)     \
	result.stripes[2] = lhs.stripes[2] OP rhs.stripes[2];
#define MATHTER_MATADD_SAME_ARRAY_4(OP) \
	MATHTER_MATADD_SAME_ARRAY_3(OP)     \
	result.stripes[3] = lhs.stripes[3] OP rhs.stripes[3];
#define MATHTER_MATADD_SAME_UNROLL(S, OP)                    \
	if (result.StripeCount == S) {                           \
		MATHTER_MATMUL_EXPAND(MATHTER_MATADD_SAME_ARRAY_##S) \
		(OP) return result;                                  \
	}


// Add & sub same layout
template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout SameLayout, bool Packed, class V = decltype(T() + U())>
inline Matrix<U, Rows, Columns, Order1, SameLayout, Packed> operator+(
	const Matrix<T, Rows, Columns, Order1, SameLayout, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, SameLayout, Packed>& rhs) {
	Matrix<U, Rows, Columns, Order1, SameLayout, Packed> result;

	MATHTER_MATADD_SAME_UNROLL(1, +);
	MATHTER_MATADD_SAME_UNROLL(2, +);
	MATHTER_MATADD_SAME_UNROLL(3, +);
	MATHTER_MATADD_SAME_UNROLL(4, +);

	for (int i = 0; i < result.StripeCount; ++i) {
		result.stripes[i] = lhs.stripes[i] + rhs.stripes[i];
	}
	return result;
}

template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout SameLayout, bool Packed, class V = decltype(T() - U())>
inline Matrix<U, Rows, Columns, Order1, SameLayout, Packed> operator-(
	const Matrix<T, Rows, Columns, Order1, SameLayout, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, SameLayout, Packed>& rhs) {
	Matrix<U, Rows, Columns, Order1, SameLayout, Packed> result;

	MATHTER_MATADD_SAME_UNROLL(1, -);
	MATHTER_MATADD_SAME_UNROLL(2, -);
	MATHTER_MATADD_SAME_UNROLL(3, -);
	MATHTER_MATADD_SAME_UNROLL(4, -);

	for (int i = 0; i < result.StripeCount; ++i) {
		result.stripes[i] = lhs.stripes[i] - rhs.stripes[i];
	}
	return result;
}


// Add & sub opposite layout
template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed, class V, class = typename std::enable_if<Layout1 != Layout2>::type>
inline Matrix<U, Rows, Columns, Order1, Layout1, Packed> operator+(
	const Matrix<T, Rows, Columns, Order1, Layout1, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs) {
	Matrix<U, Rows, Columns, Order1, Layout1, Packed> result;
	for (int i = 0; i < result.RowCount(); ++i) {
		for (int j = 0; j < result.ColumnCount(); ++j) {
			result(i, j) = lhs(i, j) + rhs(i, j);
		}
	}
	return result;
}

template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed, class V, class = typename std::enable_if<Layout1 != Layout2>::type>
inline Matrix<U, Rows, Columns, Order1, Layout1, Packed> operator-(
	const Matrix<T, Rows, Columns, Order1, Layout1, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs) {
	Matrix<U, Rows, Columns, Order1, Layout1, Packed> result;
	for (int i = 0; i < result.RowCount(); ++i) {
		for (int j = 0; j < result.ColumnCount(); ++j) {
			result(i, j) = lhs(i, j) - rhs(i, j);
		}
	}
	return result;
}



/// <summary> Performs matrix addition and stores result in this. </summary>
template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed, class V>
inline Matrix<U, Rows, Columns, Order1, Layout1, Packed>& operator+=(
	Matrix<T, Rows, Columns, Order1, Layout1, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs) {
	lhs = lhs + rhs;
	return lhs;
}

/// <summary> Performs matrix subtraction and stores result in this. </summary>
template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed, class V>
inline Matrix<U, Rows, Columns, Order1, Layout1, Packed>& operator-=(
	Matrix<T, Rows, Columns, Order1, Layout1, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs) {
	lhs = lhs - rhs;
	return lhs;
}



//------------------------------------------------------------------------------
// Matrix-Scalar arithmetic
//------------------------------------------------------------------------------


// Scalar multiplication
/// <summary> Multiplies all elements of the matrix by scalar. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
inline Matrix<T, Rows, Columns, Order, Layout, Packed>& operator*=(Matrix<T, Rows, Columns, Order, Layout, Packed>& mat, T s) {
	for (auto& stripe : mat.stripes) {
		stripe *= s;
	}
	return mat;
}
/// <summary> Divides all elements of the matrix by scalar. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
inline Matrix<T, Rows, Columns, Order, Layout, Packed>& operator/=(Matrix<T, Rows, Columns, Order, Layout, Packed>& mat, T s) {
	mat *= T(1 / s);
	return mat;
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Rows, Columns, Order, Layout, Packed> operator*(T s, const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	return mat * s;
}

template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Rows, Columns, Order, Layout, Packed> operator/(T s, const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	return mat / s;
}

template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Rows, Columns, Order, Layout, Packed> operator*(const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat, T s) {
	Matrix<T, Rows, Columns, Order, Layout, Packed> copy(mat);
	copy *= s;
	return copy;
}

template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Rows, Columns, Order, Layout, Packed> operator/(const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat, T s) {
	Matrix<T, Rows, Columns, Order, Layout, Packed> copy(mat);
	copy /= s;
	return copy;
}


//------------------------------------------------------------------------------
// Elementwise multiply and divide
//------------------------------------------------------------------------------
template <class T, class T2, int Rows, int Columns, eMatrixOrder Order, eMatrixOrder Order2, eMatrixLayout Layout, bool Packed>
auto MulElementwise(const Matrix<T, Rows, Columns, Order, Layout, Packed>& lhs, const Matrix<T2, Rows, Columns, Order2, Layout, Packed>& rhs) {
	Matrix<T, Rows, Columns, Order, Layout, Packed> result;
	for (int i = 0; i < result.StripeCount; ++i) {
		result.stripes[i] = lhs.stripes[i] * rhs.stripes[i];
	}
	return result;
}

template <class T, class T2, int Rows, int Columns, eMatrixOrder Order, eMatrixOrder Order2, eMatrixLayout Layout, bool Packed>
auto MulElementwise(const Matrix<T, Rows, Columns, Order, Layout, Packed>& lhs, const Matrix<T2, Rows, Columns, Order2, traits::OppositeLayout<Layout>::value, Packed>& rhs) {
	Matrix<T, Rows, Columns, Order, Layout, Packed> result;
	for (int i = 0; i < Rows; ++i) {
		for (int j = 0; j < Columns; ++j) {
			result(i, j) = lhs(i, j) * rhs(i, j);
		}
	}
	return result;
}

template <class T, class T2, int Rows, int Columns, eMatrixOrder Order, eMatrixOrder Order2, eMatrixLayout Layout, bool Packed>
auto DivElementwise(const Matrix<T, Rows, Columns, Order, Layout, Packed>& lhs, const Matrix<T2, Rows, Columns, Order2, Layout, Packed>& rhs) {
	Matrix<T, Rows, Columns, Order, Layout, Packed> result;
	for (int i = 0; i < result.StripeCount; ++i) {
		result.stripes[i] = lhs.stripes[i] / rhs.stripes[i];
	}
	return result;
}

template <class T, class T2, int Rows, int Columns, eMatrixOrder Order, eMatrixOrder Order2, eMatrixLayout Layout, bool Packed>
auto DivElementwise(const Matrix<T, Rows, Columns, Order, Layout, Packed>& lhs, const Matrix<T2, Rows, Columns, Order2, traits::OppositeLayout<Layout>::value, Packed>& rhs) {
	Matrix<T, Rows, Columns, Order, Layout, Packed> result;
	for (int i = 0; i < Rows; ++i) {
		for (int j = 0; j < Columns; ++j) {
			result(i, j) = lhs(i, j) / rhs(i, j);
		}
	}
	return result;
}



//------------------------------------------------------------------------------
// Unary signs
//------------------------------------------------------------------------------
template <class U, class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
inline Matrix<T, Rows, Columns, Order, Layout, Packed> operator+(const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	return Matrix<T, Rows, Columns, Order, Layout, Packed>(mat);
}

template <class U, class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
inline Matrix<T, Rows, Columns, Order, Layout, Packed> operator-(const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	return Matrix<T, Rows, Columns, Order, Layout, Packed>(mat) * T(-1);
}



} // namespace mathter