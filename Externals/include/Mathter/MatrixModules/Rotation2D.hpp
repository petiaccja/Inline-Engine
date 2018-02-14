//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixRotation2D {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
protected:
	static constexpr bool Enable2DRotation =
		(Columns == 3 && Rows == 3)
		|| (Columns == 2 && Rows == 3 && Order == eMatrixOrder::FOLLOW_VECTOR)
		|| (Columns == 3 && Rows == 2 && Order == eMatrixOrder::PRECEDE_VECTOR)
		|| (Columns == 2 && Rows == 2);
public:
	/// <summary> Creates a 2D rotation matrix. </summary>
	/// <param name="angle"> Counter-clockwise angle in radians. </param>
	static MatrixT Rotation(T angle);

	/// <summary> Sets this matrix to a 2D rotation matrix. </summary>
	/// <param name="angle"> Counter-clockwise angle in radians. </param>
	MatrixT& SetRotation(T angle) {
		*this = Rotation(angle);
		return *this;
	}
public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<Enable2DRotation, MatrixRotation2D>;
};



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto MatrixRotation2D<T, Rows, Columns, Order, Layout, Packed>::Rotation(T angle) -> MatrixT {
	MatrixT m;

	T C = cos(angle);
	T S = sin(angle);

	auto elem = [&m](int i, int j) -> T& {
		return Order == eMatrixOrder::FOLLOW_VECTOR ? m(i, j) : m(j, i);
	};

	// Indices according to follow vector order
	elem(0, 0) = C;		elem(0, 1) = S;
	elem(1, 0) = -S;	elem(1, 1) = C;

	// Rest
	for (int j = 0; j < m.ColumnCount(); ++j) {
		for (int i = (j < 2 ? 2 : 0); i < m.RowCount(); ++i) {
			m(i, j) = T(j == i);
		}
	}

	return m;
}



}