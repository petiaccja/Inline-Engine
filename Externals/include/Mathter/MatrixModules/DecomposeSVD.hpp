//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


/// <summary> A utility class that can do common operations with the singular value decomposition,
///		i.e. solving equation systems. </summary>
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class DecompositionSVD {
	template <int Rows_, int Columns_>
	using MatrixT = Matrix<T, Rows_, Columns_, Order, Layout, Packed>;

	static constexpr int Sdim = std::min(Rows, Columns);
	static constexpr int Udim = Rows;
	static constexpr int Vdim = Columns;
public:
	/// <summary> Calculates the SVD of <paramref name="arg"/> and stores it for later use. </summary>
	DecompositionSVD(const MatrixT<Rows, Columns>& arg) {
		arg.DecomposeSVD(U, S, V);
	}

	MatrixT<Udim, Sdim> U;
	MatrixT<Sdim, Sdim> S;
	MatrixT<Sdim, Vdim> V;
};



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixSVD {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }

	static constexpr int Sdim = std::min(Rows, Columns);
	static constexpr int Udim = Rows;
	static constexpr int Vdim = Columns;
public:
	/// <summary> Calculates the thin SVD of the matrix. </summary>
	/// <remarks> For wide matrices, V is wide while U and S square.
	///		For tall matrices, U is tall while S and V square. </remarks>
	void DecomposeSVD(
		Matrix<T, Udim, Sdim, Order, Layout, Packed>& Uout,
		Matrix<T, Sdim, Sdim, Order, Layout, Packed>& Sout,
		Matrix<T, Sdim, Vdim, Order, Layout, Packed>& Vout) const 
	{
		DecomposeSVD(Uout, Sout, Vout, std::integral_constant<bool, (Rows >= Columns)>());
	}

	/// <summary> Returns a helper object that works on the SVD. </summary>
	/// <remarks> You can still access U, S, and V matrices directly through the helper. </remarks>
	mathter::DecompositionSVD<T, Rows, Columns, Order, Layout, Packed> DecompositionSVD() const {
		return mathter::DecompositionSVD<T, Rows, Columns, Order, Layout, Packed>(self());
	}
private:
	void DecomposeSVD(
		Matrix<T, Rows, Columns, Order, Layout, Packed>& Uout,
		Matrix<T, Columns, Columns, Order, Layout, Packed>& Sout,
		Matrix<T, Columns, Columns, Order, Layout, Packed>& Vout,
		std::true_type) const;

	void DecomposeSVD(
		Matrix<T, Rows, Rows, Order, Layout, Packed>& Uout,
		Matrix<T, Rows, Rows, Order, Layout, Packed>& Sout,
		Matrix<T, Rows, Columns, Order, Layout, Packed>& Vout,
		std::false_type) const 
	{
		Matrix<T, Columns, Rows, Order, Layout, Packed> U;
		Matrix<T, Rows, Rows, Order, Layout, Packed> S;
		Matrix<T, Rows, Rows, Order, Layout, Packed> V;
		self().Transposed().DecomposeSVD(U, S, V);
		Vout = U.Transposed();
		Sout = S;
		Uout = V.Transposed();
	}
public:
	friend MatrixT;
	using Inherit = MatrixSVD;
};



template <class T, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void Rq2x2Helper(const Matrix<T, 2, 2, Order, Layout, Packed>& A, T& x, T& y, T& z, T& c2, T& s2) {
	T a = A(0, 0);
	T b = A(0, 1);
	T c = A(1, 0);
	T d = A(1, 1);

	if (c == 0) {
		x = a;
		y = b;
		z = d;
		c2 = 1;
		s2 = 0;
		return;
	}
	T maxden = std::max(abs(c), abs(d));

	T rcmaxden = 1/maxden;
	c *= rcmaxden;
	d *= rcmaxden;

	T den = 1/sqrt(c*c + d*d);

	T numx = (-b*c + a*d);
	T numy = (a*c + b*d);
	x = numx * den;
	y = numy * den;
	z = maxden/den;

	s2 = -c * den;
	c2 = d * den;
}


template <class T, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void Svd2x2Helper(const Matrix<T, 2, 2, Order, Layout, Packed>& A, T& c1, T& s1, T& c2, T& s2, T& d1, T& d2) {
	// Calculate RQ decomposition of A
	T x, y, z;
	Rq2x2Helper(A, x, y, z, c2, s2);

	// Calculate tangent of rotation on R[x,y;0,z] to diagonalize R^T*R
	T scaler = T(1)/std::max(abs(x), std::max(abs(y), std::numeric_limits<T>::min()));
	T x_ = x*scaler, y_ = y*scaler, z_ = z*scaler;
	T numer = ((z_-x_)*(z_+x_)) + y_*y_;
	T gamma = x_*y_;
	numer = numer == 0 ? std::numeric_limits<T>::infinity() : numer;
	T zeta = numer/gamma;

	T t = 2*impl::sign_nonzero(zeta)/(abs(zeta) + sqrt(zeta*zeta+4));

	// Calculate sines and cosines
	c1 = T(1) / sqrt(T(1) + t*t);
	s1 = c1*t;

	// Calculate U*S = R*R(c1,s1)
	T usa = c1*x - s1*y;
	T usb = s1*x + c1*y;
	T usc = -s1*z;
	T usd = c1*z;

	// Update V = R(c1,s1)^T*Q
	t = c1*c2 + s1*s2;
	s2 = c2*s1 - c1*s2;
	c2 = t;

	// Separate U and S
	d1 = std::hypot(usa, usc);
	d2 = std::hypot(usb, usd);
	T dmax = std::max(d1, d2);
	T usmax1 = d2 > d1 ? usd : usa;
	T usmax2 = d2 > d1 ? usb : -usc;

	T signd1 = impl::sign_nonzero(x*z);
	dmax *= d2 > d1 ? signd1 : 1;
	d2 *= signd1;
	T rcpdmax = 1/dmax;

	c1 = dmax != T(0) ? usmax1 * rcpdmax : T(1);
	s1 = dmax != T(0) ? usmax2 * rcpdmax : T(0);
}



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void MatrixSVD<T, Rows, Columns, Order, Layout, Packed>::DecomposeSVD(
	Matrix<T, Rows, Columns, Order, Layout, Packed>& Uout,
	Matrix<T, Columns, Columns, Order, Layout, Packed>& Sout,
	Matrix<T, Columns, Columns, Order, Layout, Packed>& Vout,
	std::true_type) const
{
	Matrix<T, Rows, Columns, Order, eMatrixLayout::COLUMN_MAJOR, false> B;
	Matrix<T, Rows, Columns, Order, eMatrixLayout::COLUMN_MAJOR, false> U;
	Matrix<T, Columns, Columns, Order, eMatrixLayout::COLUMN_MAJOR, false> V;

	// Precondition with QR if needed
	if (Rows > Columns) {
		Matrix<T, Rows, Rows, Order, Layout, Packed> Q;
		Matrix<T, Rows, Columns, Order, Layout, Packed> R;
		self().DecomposeQR(Q, R);
		B = R;
		U = Q.template Submatrix<Rows, Columns>(0, 0);
		V.SetIdentity();
	}
	else {
		B = self();
		U.SetIdentity();
		V.SetIdentity();
	}

	T tolerance = T(1e-6);

	T N = B.NormSquared();
	T s;

	do {
		s = 0;

		for (int i = 0; i < self().ColumnCount(); ++i) {
			for (int j = i+1; j < self().ColumnCount(); ++j) {
				s += B(i, j)*B(i, j) + B(j, i)*B(j, i);

				T s1, c1, s2, c2, d1, d2; // SVD of the submat row,col i,j of B
				Matrix<T, 2, 2, eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout::ROW_MAJOR, false> Bsub = {
					B(i, i), B(i, j),
					B(j, i), B(j, j)
				};
				Svd2x2Helper(Bsub, c1, s1, c2, s2, d1, d2);

				// Apply givens rotations given by 2x2 SVD to working matrices
				// B = R(c1,s1)*B*R(c2,-s2)
				Vector<T, 4, false> givensCoeffs = { c1, -s1, s1, c1 };
				Vector<T, 4, false> bElems;
				for (int col = 0; col < B.ColumnCount(); ++col) {
					bElems.Set(B(i, col), B(j, col), B(i, col), B(j, col));
					bElems *= givensCoeffs;
					B(i, col) = bElems(0) + bElems(1);
					B(j, col) = bElems(2) + bElems(3);
				}
				auto coli = B.stripes[i];
				B.stripes[i] = c2*coli + -s2*B.stripes[j];
				B.stripes[j] = s2*coli + c2*B.stripes[j];

				// U = U*R(c1,s1);
				coli = U.stripes[i];
				U.stripes[i] = c1*coli + -s1*U.stripes[j];
				U.stripes[j] = s1*coli + c1*U.stripes[j];

				// V = V*R(c2,s2);
				auto coliv = V.stripes[i];
				V.stripes[i] = c2*coliv + -s2*V.stripes[j];
				V.stripes[j] = s2*coliv + c2*V.stripes[j];
			}
		}
	} while (s > tolerance*N);

	Uout = U;
	Sout.SetZero();
	for (int i = 0; i<B.ColumnCount(); ++i) {
		Sout(i, i) = B(i, i);
	}
	Vout = V.Transposed();
}


}