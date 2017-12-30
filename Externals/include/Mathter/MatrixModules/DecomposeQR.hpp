//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class DecompositionQR {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
public:
	DecompositionQR(const MatrixT& arg) {

	}

	MatrixT U, S, V;
};



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixQR {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
	static constexpr bool EnableQR = Rows >= Columns;
public:
	void DecomposeQR(
		Matrix<T, Rows, Rows, Order, Layout, Packed>& Q,
		Matrix<T, Rows, Columns, Order, Layout, Packed>& R) const;

public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<EnableQR, MatrixQR>;
};



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void MatrixQR<T, Rows, Columns, Order, Layout, Packed>::DecomposeQR(
	Matrix<T, Rows, Rows, Order, Layout, Packed>& Q,
	Matrix<T, Rows, Columns, Order, Layout, Packed>& R) const
{
	R = self();
	Q.SetIdentity();

	Matrix<T, Rows, Rows, Order, Layout, false> Qi;
	Vector<T, Rows, false> u;
	Matrix<T, Rows, 1, Order, eMatrixLayout::ROW_MAJOR, false> v;

	for (int col = 0; col<self().ColumnCount(); ++col) {
		u = R.Column(col);
		for (int i = 0; i<col; ++i) {
			u(i) = T(0);
		}
		T alpha = impl::sign(R(col, col)) * u.LengthPrecise();
		u(col) -= alpha;
		T norm = u.LengthPrecise();
		if (norm == 0) {
			continue;
		}
		u /= norm;
		v = u;
		Qi = (T(-2)*v)*v.Transposed();
		for (int i = 0; i<Q.ColumnCount(); ++i) {
			Qi(i, i) += T(1);
		}
		R = Qi*R;
		Q = Qi*Q;
	}
	Q = Q.Transposed();
}



}