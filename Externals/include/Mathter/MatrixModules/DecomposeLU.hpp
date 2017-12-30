//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class DecompositionLU {
	using MatrixT = Matrix<T, Dim, Dim, Order, Layout, Packed>;
public:
	DecompositionLU(const MatrixT& arg) {
		arg.DecomposeLU(L, U);
		T prod = L(0, 0);
		T sum = std::abs(prod);
		for (int i = 1; i < Dim; ++i) {
			prod *= L(i, i);
			sum += std::abs(L(i, i));
		}
		sum /= Dim;
		solvable = std::abs(prod) / sum > T(1e-6);
	}

	bool Solve(Vector<float, Dim, Packed>& x, const Vector<T, Dim, Packed>& b);

	MatrixT L, U;
	bool solvable;
};


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixLU {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
public:
	friend MatrixT;
	using Inherit = impl::Empty<MatrixLU>;
};


template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixLU<T, Dim, Dim, Order, Layout, Packed> {
	using MatrixT = Matrix<T, Dim, Dim, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	void DecomposeLU(MatrixT& L, MatrixT& U) const;
	mathter::DecompositionLU<T, Dim, Order, Layout, Packed> DecompositionLU() const {
		return mathter::DecompositionLU<T, Dim, Order, Layout, Packed>(self());
	}
public:
	friend MatrixT;
	using Inherit = MatrixLU;
};



// From: https://www.gamedev.net/resources/_/technical/math-and-physics/matrix-inversion-using-lu-decomposition-r3637
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void MatrixLU<T, Dim, Dim, Order, Layout, Packed>::DecomposeLU(MatrixT& L, MatrixT& U) const {
	const auto& A = self();
	constexpr int n = Dim;

	for (int i = 0; i < n; ++i) {
		for (int j = i + 1; j < n; ++j) {
			L(i, j) = 0;
		}
		for (int j = 0; j <= i; ++j) {
			U(i, j) = i == j;
		}
	}

	// Crout's algorithm
	for (int i = 0; i < n; ++i) {
		L(i, 0) = A(i, 0);
	}
	for (int j = 1; j < n; ++j) {
		U(0, j) = A(0, j) / L(0, 0);
	}

	for (int j = 1; j < n-1; ++j) {
		for (int i = j; i < n; ++i) {
			float Lij;
			Lij = A(i, j);
			for (int k = 0; k <= j - 1; ++k) {
				Lij -= L(i, k)*U(k, j);
			}
			L(i, j) = Lij;
		}
		for (int k = j; k < n; ++k) {
			float Ujk;
			Ujk = A(j, k);
			for (int i = 0; i <= j - 1; ++i) {
				Ujk -= L(j, i)*U(i, k);
			}
			Ujk /= L(j, j);
			U(j, k) = Ujk;
		}
	}

	L(n - 1, n - 1) = A(n - 1, n - 1);
	for (int k = 0; k < n - 1; ++k) {
		L(n - 1, n - 1) -= L(n - 1, k)*U(k, n - 1);
	}
}


template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
bool DecompositionLU<T, Dim, Order, Layout, Packed>::Solve(Vector<float, Dim, Packed>& x, const Vector<T, Dim, Packed>& b) {
	if (!solvable) {
		for (int i = 0; i < Dim; ++i) {
			x(i) = T(0);
		}
		return false;
	}

	// Solve Ld = b
	Matrix<T, Dim, Dim + 1, eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout::ROW_MAJOR, Packed> L_b;

	for (int i = 0; i < Dim; ++i) {
		for (int j = 0; j < Dim; ++j) {
			L_b(i, j) = L(i, j);
		}
		L_b(i, Dim) = b(i);
	}

	for (int i = 0; i < Dim - 1; ++i) {
		for (int i2 = i + 1; i2 < Dim; ++i2) {
			L_b.stripes[i] /= L_b(i, i);
			T coeff = L_b(i2, i);
			L_b.stripes[i2] -= L_b.stripes[i] * coeff;
		}
	}
	L_b.stripes[Dim - 1] /= L_b(Dim - 1, Dim - 1);

	// Solve Ux = d
	Matrix<T, Dim, Dim + 1, eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout::ROW_MAJOR, Packed> U_d;
	for (int i = 0; i < Dim; ++i) {
		for (int j = 0; j < Dim; ++j) {
			U_d(i, j) = U(i, j);
		}
		U_d(i, Dim) = L_b(i, Dim);
	}

	// only works for Crout's algorithm, where U's diagonal is 1s
	for (int i = Dim - 1; i > 0; --i) {
		for (int i2 = i - 1; i2 >= 0; --i2) {
			T coeff = U_d(i2, i);
			U_d.stripes[i2] -= U_d.stripes[i] * coeff;
		}
	}

	// Output resulting vector
	for (int i = 0; i < Dim; ++i) {
		x(i) = U_d(i, Dim);
	}

	return true;
}


}