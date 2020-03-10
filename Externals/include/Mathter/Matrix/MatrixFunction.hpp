#pragma once

#include "MatrixImpl.hpp"


namespace mathter {


/// <summary> Returns the trace (sum of diagonal elements) of the matrix. </summary>
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
T Trace(const Matrix<T, Dim, Dim, Order, Layout, Packed>& m) {
	T sum = m(0, 0);
	for (int i = 1; i < Dim; ++i) {
		sum += m(i, i);
	}
	return sum;
}

/// <summary> Returns the determinant of the matrix. </summary>
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
T Determinant(const Matrix<T, Dim, Dim, Order, Layout, Packed>& m) {
	// only works if L's diagonal is 1s
	int parity;
	auto [L, U, P] = DecomposeLUP(m, parity);
	T prod = U(0, 0);
	for (int i = 1; i < U.RowCount(); ++i) {
		prod *= U(i, i);
	}
	return parity * prod;
}

/// <summary> Transposes the matrix in-place. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Columns, Rows, Order, Layout, Packed> Transpose(const Matrix<T, Rows, Columns, Order, Layout, Packed>& m) {
	Matrix<T, Columns, Rows, Order, Layout, Packed> result;
	for (int i = 0; i < m.RowCount(); ++i) {
		for (int j = 0; j < m.ColumnCount(); ++j) {
			result(j, i) = m(i, j);
		}
	}
	return result;
}

/// <summary> Returns the inverse of the matrix. </summary>
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Dim, Dim, Order, Layout, Packed> Inverse(const Matrix<T, Dim, Dim, Order, Layout, Packed>& m) {
	Matrix<T, Dim, Dim, Order, Layout, Packed> ret;

	auto LUP = DecomposeLUP(m);

	Vector<T, Dim, Packed> b(0);
	Vector<T, Dim, Packed> x;
	for (int col = 0; col < Dim; ++col) {
		b(std::max(0, col - 1)) = 0;
		b(col) = 1;
		x = LUP.Solve(b);
		for (int i = 0; i < Dim; ++i) {
			ret(i, col) = x(i);
		}
	}

	return ret;
}

/// <summary> Calculates the square of the Frobenius norm of the matrix. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
T NormSquared(const Matrix<T, Rows, Columns, Order, Layout, Packed>& m) {
	T sum = T(0);
	for (auto& stripe : m.stripes) {
		sum += LengthSquared(stripe);
	}
	sum /= (m.RowCount() * m.ColumnCount());
	return sum;
}

/// <summary> Calculates the Frobenius norm of the matrix. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
T Norm(const Matrix<T, Rows, Columns, Order, Layout, Packed>& m) {
	return sqrt(NormSquared(m));
}



} // namespace mathter