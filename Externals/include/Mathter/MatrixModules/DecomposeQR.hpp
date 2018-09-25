//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


/// <summary> A utility class that can do common operations with the QR decomposition,
///		i.e. solving equation systems. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class DecompositionQR {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
public:
	/// <summary> Calculates the QR decomposition of <paramref name="arg"/> 
	///		and stores it for later calculations. </summary>
	DecompositionQR(const MatrixT& arg) {
		arg.DecomposeQR(Q, R);
	}

	Matrix<T, Rows, Rows, Order, Layout, Packed> Q;
	Matrix<T, Rows, Columns, Order, Layout, Packed> R;
};



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixQR {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
	static constexpr bool EnableQR = Rows >= Columns;
public:
	/// <summary> Calculates the QR decomposition of the matrix using Householder transforms. </summary>
	/// <remarks> The matrix must have Rows &lt;= Columns. It's a full QR decomposition, not a thin one. </remarks>
	void DecomposeQR(
		Matrix<T, Rows, Rows, Order, Layout, Packed>& Q,
		Matrix<T, Rows, Columns, Order, Layout, Packed>& R) const;

	/// <summary> Returns a helper object that works on the QR decomposition. </summary>
	/// <remarks> You can still access Q and R matrices directly through the helper. </remarks>
	mathter::DecompositionQR<T, Rows, Columns, Order, Layout, Packed> DecompositionQR() const {
		return mathter::DecompositionQR<T, Rows, Columns, Order, Layout, Packed>(self());
	}
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

	Matrix<T, Rows, Rows, Order, Layout, Packed> Qi;
	Vector<T, Rows, Packed> u;
	Matrix<T, Rows, 1, Order, eMatrixLayout::ROW_MAJOR, Packed> v;

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