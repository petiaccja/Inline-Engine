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

	/// <summary> Rotates around coordinate axis. </summary>
	/// <typeparam name="Axis"> 0 for X, 1 for Y, 2 for Z and so on... </typeparam>
	/// <param name="angle"> Angle of rotation in radians. </param>
	/// <remarks> Positive angles rotate according to the right-hand rule in right-handed
	///		coordinate systems (left-handed likewise).
	template <int Axis>
	static MatrixT RotationAxis(T angle);

	/// <summary> Rotates around the X axis according to the right (left) hand rule. </summary>
	/// <param name="angle"> Angle of rotation in radians. </param>
	static MatrixT RotationX(T angle);

	/// <summary> Rotates around the Y axis according to the right (left) hand rule. </summary>
	/// <param name="angle"> Angle of rotation in radians. </param>
	static MatrixT RotationY(T angle);

	/// <summary> Rotates around the Z axis according to the right (left) hand rule. </summary>
	/// <param name="angle"> Angle of rotation in radians. </param>
	static MatrixT RotationZ(T angle);

	/// <summary> Rotates around three axes in succession. </summary>
	/// <remarks> Axes: 0 for X, 1 for Y and 2 for Z.
	///		Angles in radians. </remarks>
	template <int FirstAxis, int SecondAxis, int ThirdAxis>
	static MatrixT RotationAxis3(T angle1, T angle2, T angle3);

	/// <summary> Rotation matrix from Euler angles. Rotations are Z-X-Z. </summary>
	/// <param name="z1"> Angle of the first rotation around Z in radians. </param>
	/// <param name="x2"> Angle of the second rotation around X in radians. </param>
	/// <param name="z3"> Angle of the third rotation around Z in radians. </param>
	static MatrixT RotationEuler(float z1, float x2, float z3);

	/// <summary> Rotation matrix from roll-pitch-yaw angles. Rotations are X-Y-Z. </summary>
	/// <param name="z1"> Angle of the first rotation around X in radians. </param>
	/// <param name="x2"> Angle of the second rotation around Y in radians. </param>
	/// <param name="z3"> Angle of the third rotation around Z in radians. </param>
	static MatrixT RotationRPY(float x1, float y2, float z3);

	/// <summary> Rotates around an arbitrary axis. </summary>
	/// <param name="axis"> Axis of rotation, must be normalized. </param>
	/// <param name="angle"> Angle of rotation in radians. </param>
	/// <remarks> Right-hand (left-hand) rule is followed in right-handed (left-handed) systems. </remarks>
	template <class U, bool Vpacked>
	static MatrixT RotationAxisAngle(const Vector<U, 3, Vpacked>& axis, T angle);


	// Member rotation

	/// <summary> Rotates around coordinate axis. </summary>
	/// <typeparam name="Axis"> 0 for X, 1 for Y, 2 for Z and so on... </typeparam>
	/// <param name="angle"> Angle of rotation in radians. </param>
	/// <remarks> Positive angles rotate according to the right-hand rule in right-handed
	///		coordinate systems (left-handed likewise).
	template <int Axis>
	MatrixT& SetRotationAxis(T angle) { self() = RotationAxis<Axis>(angle); return self(); }

	/// <summary> Rotates around the X axis according to the right (left) hand rule. </summary>
	/// <param name="angle"> Angle of rotation in radians. </param>
	MatrixT& SetRotationX(T angle) { self() = RotationX(angle); return self(); }

	/// <summary> Rotates around the Y axis according to the right (left) hand rule. </summary>
	/// <param name="angle"> Angle of rotation in radians. </param>
	MatrixT& SetRotationY(T angle) { self() = RotationY(angle); return self(); }

	/// <summary> Rotates around the Z axis according to the right (left) hand rule. </summary>
	/// <param name="angle"> Angle of rotation in radians. </param>
	MatrixT& SetRotationZ(T angle) { self() = RotationZ(angle); return self(); }

	/// <summary> Rotates around three axes in succession. </summary>
	/// <remarks> Axes: 0 for X, 1 for Y and 2 for Z.
	///		Angles in radians. </remarks>
	template <int FirstAxis, int SecondAxis, int ThirdAxis>
	MatrixT& SetRotationAxis3(T angle1, T angle2, T angle3) { self() = RotationAxis3<FirstAxis, SecondAxis, ThirdAxis>(angle1, angle2, angle3); return self(); }

	/// <summary> Rotation matrix from Euler angles. Rotations are Z-X-Z. </summary>
	/// <param name="z1"> Angle of the first rotation around Z in radians. </param>
	/// <param name="x2"> Angle of the second rotation around X in radians. </param>
	/// <param name="z3"> Angle of the third rotation around Z in radians. </param>
	MatrixT& SetRotationEuler(float z1, float x2, float z3) { self() = RotationEuler(z1, x2, z3); return self(); }

	/// <summary> Rotation matrix from roll-pitch-yaw angles. Rotations are X-Y-Z. </summary>
	/// <param name="z1"> Angle of the first rotation around X in radians. </param>
	/// <param name="x2"> Angle of the second rotation around Y in radians. </param>
	/// <param name="z3"> Angle of the third rotation around Z in radians. </param>
	MatrixT& SetRotationRPY(float x1, float y2, float z3) { self() = RotationRPY(x1, y2, z3); return self(); }

	/// <summary> Rotates around an arbitrary axis. </summary>
	/// <param name="axis"> Axis of rotation, must be normalized. </param>
	/// <param name="angle"> Angle of rotation in radians. </param>
	/// <remarks> Right-hand (left-hand) rule is followed in right-handed (left-handed) systems. </remarks>
	template <class U, bool Vpacked>
	MatrixT& SetRotationAxisAngle(const Vector<U, 3, Vpacked>& axis, T angle) { self() = Rotation(axis, angle); return self(); }

	/// <summary> Determines if the matrix is a proper rotation matrix. </summary>
	/// <remarks> Proper rotation matrices are orthogonal and have a determinant of +1. </remarks>
	bool IsRotationMatrix3D() const {
		Vector<T, 3> rows[3] = {
			{ self()(0,0), self()(0,1), self()(0,2) },
			{ self()(1,0), self()(1,1), self()(1,2) },
			{ self()(2,0), self()(2,1), self()(2,2) },
		};
		return (std::abs(Dot(rows[0], rows[1])) + std::abs(Dot(rows[0], rows[2])) + std::abs(Dot(rows[1], rows[2]))) < T(0.0005) // rows are orthogonal to each other
			&& rows[0].IsNormalized() && rows[1].IsNormalized() && rows[2].IsNormalized() // all rows are normalized
			&& Matrix<T, 3, 3, Order, Layout, Packed>(self().template Submatrix<3,3>(0,0)).Determinant() > 0; // not an improper rotation
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
	using RotMat = Matrix<T, 3, 3, eMatrixOrder::FOLLOW_VECTOR, Layout, Packed>;
	Matrix<T, 3, 1, eMatrixOrder::FOLLOW_VECTOR, Layout, Packed> u(axis(0), axis(1), axis(2));
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