//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixRotation3D {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
protected:
	static constexpr bool Enable3DRotation =
		(Columns == 4 && Rows == 4)
		|| (Columns == 3 && Rows == 4 && Order == eMatrixOrder::FOLLOW_VECTOR)
		|| (Columns == 4 && Rows == 3 && Order == eMatrixOrder::PRECEDE_VECTOR)
		|| (Columns == 3 && Rows == 3);
public:
	// Static rotation
	template <int Axis>
	static MatrixT RotationAxis(T angle);

	static MatrixT RotationX(T angle);
	static MatrixT RotationY(T angle);
	static MatrixT RotationZ(T angle);

	template <int FirstAxis, int SecondAxis, int ThirdAxis>
	static MatrixT RotationAxis3(T angle1, T angle2, T angle3);

	static MatrixT RotationEuler(float z1, float x2, float z3);
	static MatrixT RotationRPY(float x1, float y2, float z3);
	template <class U, bool Vpacked>
	static MatrixT RotationAxisAngle(const Vector<U, 3, Vpacked>& axis, T angle);

	// Member rotation
	template <int Axis>
	MatrixT& SetRotationAxis(T angle) { self() = RotationAxis<Axis>(angle); return self(); }

	MatrixT& SetRotationX(T angle) { self() = RotationX(angle); return self(); }
	MatrixT& SetRotationY(T angle) { self() = RotationY(angle); return self(); }
	MatrixT& SetRotationZ(T angle) { self() = RotationZ(angle); return self(); }

	template <int FirstAxis, int SecondAxis, int ThirdAxis>
	MatrixT& SetRotationAxis3(T angle1, T angle2, T angle3) { self() = RotationAxis3<FirstAxis, SecondAxis, ThirdAxis>(angle1, angle2, angle3); return self(); }

	MatrixT& SetRotationEuler(float z1, float x2, float z3) { self() = RotationEuler(z1, x2, z3); return self(); }
	MatrixT& SetRotationRPY(float x1, float y2, float z3) { self() = RotationRPY(x1, y2, z3); return self(); }
	template <class U, bool Vpacked>
	MatrixT& SetRotationAxisAngle(const Vector<U, 3, Vpacked>& axis, T angle) { self() = Rotation(axis, angle); return self(); }

	bool IsRotationMatrix3D() const {
		Vector<T, 3> rows[3] = {
			{ self()(0,0), self()(0,1), self()(0,2) },
		{ self()(1,0), self()(1,1), self()(1,2) },
		{ self()(2,0), self()(2,1), self()(2,2) },
		};
		return (std::abs(Dot(rows[0], rows[1])) + std::abs(Dot(rows[0], rows[2])) + std::abs(Dot(rows[1], rows[2]))) < T(0.0001)
			&& rows[0].IsNormalized() && rows[1].IsNormalized() && rows[2].IsNormalized();
	}
public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<Enable3DRotation, MatrixRotation3D>;
};



template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
template <int Axis>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationAxis(T angle) -> MatrixT
{
	MatrixT m;

	T C = cos(angle);
	T S = sin(angle);

	auto elem = [&m](int i, int j) -> T& {
		return Order == eMatrixOrder::FOLLOW_VECTOR ? m(i, j) : m(j, i);
	};

	static_assert(0 <= Axis && Axis < 3, "You may choose X=0, Y=1 or Z=2 axes.");

	// Indices according to follow vector order
	if (Axis == 0) {
		// Rotate around X
		elem(0, 0) = 1;		elem(0, 1) = 0;		elem(0, 2) = 0;
		elem(1, 0) = 0;		elem(1, 1) = C;		elem(1, 2) = S;
		elem(2, 0) = 0;		elem(2, 1) = -S;	elem(2, 2) = C;
	}
	else if (Axis == 1) {
		// Rotate around Y
		elem(0, 0) = C;		elem(0, 1) = 0;		elem(0, 2) = -S;
		elem(1, 0) = 0;		elem(1, 1) = 1;		elem(1, 2) = 0;
		elem(2, 0) = S;		elem(2, 1) = 0;		elem(2, 2) = C;
	}
	else {
		// Rotate around Z
		elem(0, 0) = C;		elem(0, 1) = S;		elem(0, 2) = 0;
		elem(1, 0) = -S;	elem(1, 1) = C;		elem(1, 2) = 0;
		elem(2, 0) = 0;		elem(2, 1) = 0;		elem(2, 2) = 1;
	}

	// Rest
	for (int j = 0; j < m.ColumnCount(); ++j) {
		for (int i = (j < 3 ? 3 : 0); i < m.RowCount(); ++i) {
			m(i, j) = T(j == i);
		}
	}

	return m;
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationX(T angle)-> MatrixT
{
	return RotationAxis<0>(angle);
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationY(T angle) -> MatrixT
{
	return RotationAxis<1>(angle);
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationZ(T angle) -> MatrixT
{
	return RotationAxis<2>(angle);
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
template <int Axis1, int Axis2, int Axis3>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationAxis3(T angle1, T angle2, T angle3) -> MatrixT
{
	return RotationAxis<Axis1>(angle1) * RotationAxis<Axis2>(angle2) * RotationAxis<Axis3>(angle3);
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationEuler(float z1, float x2, float z3) -> MatrixT
{
	return RotationAxis3<2, 0, 2>(z1, x2, z3);
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationRPY(float x1, float y2, float z3) -> MatrixT
{
	return RotationAxis3<0, 1, 2>(x1, y2, z3);
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
template <class U, bool Vpacked>
auto MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::RotationAxisAngle(const Vector<U, 3, Vpacked>& axis, T angle) -> MatrixT
{
	assert(axis.IsNormalized());
	MatrixT m;

	T C = cos(angle);
	T S = sin(angle);

	// 3x3 rotation sub-matrix
	using RotMat = Matrix<T, 3, 3, eMatrixOrder::FOLLOW_VECTOR>;
	Matrix<T, 3, 1, eMatrixOrder::FOLLOW_VECTOR> u(axis(0), axis(1), axis(2));
	RotMat cross = {
		0, -u(2), u(1),
		u(2), 0, -u(0),
		-u(1), u(0), 0
	};
	RotMat rot = C*RotMat::Identity() + S*cross + (1 - C)*(u*u.Transposed());


	// Elements
	auto elem = [&m](int i, int j) -> T& {
		return Order == eMatrixOrder::PRECEDE_VECTOR ? m(i, j) : m(j, i);
	};
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 3; ++i) {
			elem(i, j) = rot(i, j);
		}
	}

	// Rest
	for (int j = 0; j < m.Width(); ++j) {
		for (int i = (j < 3 ? 3 : 0); i < m.Height(); ++i) {
			m(i, j) = T(j == i);
		}
	}

	return m;
}


}