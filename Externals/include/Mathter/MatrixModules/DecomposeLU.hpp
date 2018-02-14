//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


/// <summary> A utility class that can do common operations with the LU decomposition,
///		i.e. solving equation systems. </summary>
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class DecompositionLU {
	using MatrixT = Matrix<T, Dim, Dim, Order, Layout, Packed>;

	template <class T2, int Dim2, eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	friend class DecompositionLUP;
private:
	static Vector<float, Dim, Packed> Solve(const MatrixT& L, const MatrixT& U, const Vector<T, Dim, Packed>& b);
public:
	/// <summary> Calculates the LU decomposition of <paramref name="arg"/>
	///		and stores it for later calculations. </summary>
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

	/// <summary> Solves the equation system Ax=b, that is LUx=b. </summary>
	/// <remarks> If the equation is singular or the LU decomposition fails, garbage is returned. </remarks>
	/// <param name="b"> The right hand side vector. </summary>
	/// <returns> The solution x. </returns>
	Vector<float, Dim, Packed> Solve(const Vector<T, Dim, Packed>& b) const {
		return Solve(L, U, b);
	}

	MatrixT L, U;
	bool solvable;
};


/// <summary> A utility class that can do common operations with the LUP decomposition,
///		i.e. solving equation systems. </summary>
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class DecompositionLUP {
	using MatrixT = Matrix<T, Dim, Dim, Order, Layout, Packed>;
public:
	/// <summary> Calculates the LUP decomposition of <paramref name="arg"/>
	///		and stores it for later calculations. </summary>
	DecompositionLUP(const MatrixT& arg) {
		arg.DecomposeLUP(L, U, P);
		T prod = L(0, 0);
		T sum = std::abs(prod);
		for (int i = 1; i < Dim; ++i) {
			prod *= L(i, i);
			sum += std::abs(L(i, i));
		}
		sum /= Dim;
		solvable = std::abs(prod) / sum > T(1e-6);
	}

	/// <summary> Solves the equation system Ax=b, that is LUx=Pb. </summary>
	/// <remarks> If the equation is singular garbage is returned. </remarks>
	/// <param name="b"> The right hand side vector. </param>
	/// <returns> The solution x. </returns>
	Vector<float, Dim, Packed> Solve(const Vector<T, Dim, Packed>& b) const;

	MatrixT L, U;
	Vector<int, Dim, false> P;
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
	/// <summary> Decomposes the matrix into the product of a lower and an upper triangular matrix. </summary>
	/// <remarks> Does not use pivoting and it may fail on non-singular matrices as well. Faster than LUP. </remarks>
	void DecomposeLU(MatrixT& L, MatrixT& U) const;

	/// <summary> Return a helper object that operates on the LU decomposition of this matrix. </summary>
	/// <remarks> You can still access the L and U matrices though the helper. </remarks>
	mathter::DecompositionLU<T, Dim, Order, Layout, Packed> DecompositionLU() const {
		return mathter::DecompositionLU<T, Dim, Order, Layout, Packed>(self());
	}

	/// <summary> Implements LU decomposition with partial pivoting. </summary>
	/// <remarks> Handles singular matrices as well. </remarks>
	/// <param name="L"> Lower triangular matrix, LU=P'A. </param>
	/// <param name="U"> Upper triangular matrix, LU=P'A. </param>
	/// <param name="P"> Row permutations. LU=P'A, where P' is a matrix whose i-th row's P[i]-th element is one. </param>
	void DecomposeLUP(MatrixT& L, MatrixT& U, Vector<int, Dim, false>& P) const;

	/// <summary> Implements LU decomposition with partial pivoting. </summary>
	/// <remarks> Handles singular matrices as well. </remarks>
	/// <param name="L"> Lower triangular matrix, LU=P'A. </param>
	/// <param name="U"> Upper triangular matrix, LU=P'A. </param>
	/// <param name="P"> Row permutations. LU=P'A, where P' is a matrix whose i-th row's P[i]-th element is one. </param>
	/// <param name="parity"> The parity of the permutation described by P. Odd: 1, Even: -1. </param>
	void DecomposeLUP(MatrixT& L, MatrixT& U, Vector<int, Dim, false>& P, int& parity) const;

	/// <summary> Return a helper object that operates on the LUP decomposition of this matrix. </summary>
	/// <remarks> You can still access the L and U matrices and P permutation though the helper. </remarks>
	mathter::DecompositionLUP<T, Dim, Order, Layout, Packed> DecompositionLUP() const {
		return mathter::DecompositionLUP<T, Dim, Order, Layout, Packed>(self());
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
void MatrixLU<T, Dim, Dim, Order, Layout, Packed>::DecomposeLUP(MatrixT& L, MatrixT& U, Vector<int, Dim, false>& P) const {
	int ignore;
	return DecomposeLUP(L, U, P, ignore);
}

template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void MatrixLU<T, Dim, Dim, Order, Layout, Packed>::DecomposeLUP(MatrixT& L, MatrixT& U, Vector<int, Dim, false>& P, int& parity) const {
	using impl::Range;

	U = self();

	int n = self().RowCount();
	parity = 1;

	for (int i : Range(0, n)) {
		P(i) = i;
	}

	for (int j : Range(0,n)) {
		// find largest pivot elements
		T p = 0;
		int largest;
		for (int i : Range(j,n)) {
			if (std::abs(U(i,j)) > p) {
				largest = i;
				p = std::abs(U(i, j));
			}
		}

		// if pivot is zero TODO
		if (p == 0) {
			continue;
		}

		// swap rows to move pivot to top row
		std::swap(P(j), P(largest));
		parity *= (j != largest ? -1 : 1);
		for (int i : Range(0,n)) {
			std::swap(U(j, i), U(largest, i));
		}

		// do some magic
		for (int i : Range(j+1, n)) {
			U(i, j) = U(i, j)/U(j, j);
			for (int k : Range(j+1, n)) {
				U(i, k) = U(i, k) - U(i, j)*U(j, k);
			}
		}
	}

	// copy elements to L
	for (int j : Range(0, n)) {
		for (int i : Range(j+1, n)) {
			L(i, j) = U(i, j);
			U(i, j) = T(0);
			L(j, i) = T(0);
		}
	}
	for (int i : Range(n)) {
		L(i, i) = 1;
	}
}


template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Vector<float, Dim, Packed> DecompositionLU<T, Dim, Order, Layout, Packed>::Solve(const MatrixT& L, const MatrixT& U, const Vector<T, Dim, Packed>& b) {
	// Matrix to do gaussian elimination with
	Matrix<T, Dim, Dim + 1, eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout::ROW_MAJOR, Packed> E;

	// Solve Ld = b;
	E.template Submatrix<Dim, Dim>(0, 0) = L;
	E.Column(Dim) = b;

	for (int i = 0; i < Dim - 1; ++i) {
		for (int i2 = i + 1; i2 < Dim; ++i2) {
			E.stripes[i] /= E(i, i);
			T coeff = E(i2, i);
			E.stripes[i2] -= E.stripes[i] * coeff;
		}
	}
	E(Dim - 1, Dim) /= E(Dim - 1, Dim - 1);
	// d is now the last column of E

	// Solve Ux = d
	E.template Submatrix<Dim, Dim>(0, 0) = U;

	for (int i = Dim - 1; i > 0; --i) {
		for (int i2 = i - 1; i2 >= 0; --i2) {
			E.stripes[i] /= E(i, i);
			T coeff = E(i2, i);
			E.stripes[i2] -= E.stripes[i] * coeff;
		}
	}
	E(0, Dim) /= E(0, 0);
	// x is now the last column of E

	return E.Column(Dim);
}


template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Vector<float, Dim, Packed> DecompositionLUP<T, Dim, Order, Layout, Packed>::Solve(const Vector<T, Dim, Packed>& b) const {
	using impl::Range;

	// Permute b
	Vector<T, Dim, Packed> bp;
	for (int i : Range(0, P.Dimension())) {
		bp(i) = b(P(i));
	}

	// Solve
	auto x = DecompositionLU<T, Dim, Order, Layout, Packed>::Solve(L, U, bp);

	return x;
}


}