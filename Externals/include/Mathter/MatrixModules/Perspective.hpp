//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixPerspective {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
public:
	friend MatrixT;
	using Inherit = impl::Empty<MatrixPerspective>;
};

template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixProjectiveBase {
protected:
	using MatrixT = Matrix<T, Dim, Dim, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	/// <summary> Creates a perspective projection matrix. </summary>
	/// <param name="fovX"> Field of view on the first axis (usually denoted X) in radians. </param>
	/// <param name="ratios"> Aspect ratio (or ratios in higher dimensions). FovX/FovY. </param>
	/// <param name="nearPlane"> Near bound of the projected volume on the last axis (Z in 3D). </param>
	/// <param name="farPlane"> Far bound of the projected volume on the last axis (Z in 3D). </param>
	/// <param name="projNearPlane"> The near plane is taken here after projection. </param>
	/// <param name="projFarPlane"> The far plane is taken here after projection. </param>
	/// <remarks> Post-projection near and far planes can be inverted. Negative ratios invert image. </remarks>
	static MatrixT Perspective(T fovX, const Vector<T, Dim-2, Packed>& ratios, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1) {
		assert((nearPlane < 0 && farPlane < nearPlane) || (0 < nearPlane && nearPlane < farPlane));

		MatrixT m;
		m.SetZero();
		// Layout be like (precede_vector):
		// w 0 0 0
		// 0 h 0 0
		// 0 0 A B
		// 0 0 C 0
		fovX = std::abs(fovX);
		T N = nearPlane;
		T F = farPlane;
		T n = projNearPlane;
		T f = projFarPlane;
		T C = nearPlane < T(0) ? T(-1) : T(1);
		T A = C*(f*F - n*N) / (F - N);
		T B = C*F*N*(n - f) / (F - N);
		Vector<T, Dim-2, Packed> adjRatios = ratios(0) / ratios;
		T w = tan(T(0.5)*fovX);
		adjRatios /= w;
		for (int i = 0; i < adjRatios.Dimension(); ++i) {
			m(i, i) = adjRatios(i);
		}
		m(m.RowCount() - 2, m.ColumnCount() - 2) = A;
		if (Order == eMatrixOrder::FOLLOW_VECTOR) {
			std::swap(B, C);
		}
		m(m.RowCount() - 2, m.ColumnCount() - 1) = B;
		m(m.RowCount() - 1, m.ColumnCount() - 2) = C;
		return m;
	}

	/// <summary> Set this matrix to a perspective projection matrix. </summary>
	/// <param name="fovX"> Field of view on the first axis (usually denoted X) in radians. </param>
	/// <param name="ratios"> Aspect ratio (or ratios in higher dimensions). FovX/FovY. </param>
	/// <param name="nearPlane"> Near bound of the projected volume on the last axis (Z in 3D). </param>
	/// <param name="farPlane"> Far bound of the projected volume on the last axis (Z in 3D). </param>
	/// <param name="projNearPlane"> The near plane is taken here after projection. </param>
	/// <param name="projFarPlane"> The far plane is taken here after projection. </param>
	/// <remarks> Post-projection near and far planes can be inverted. Negative ratios invert image. </remarks>
	MatrixT& SetPerspective(T fovX, const Vector<T, Dim - 2, Packed>& ratios, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1) {
		self() = Perspective(fovX, ratios, nearPlane, farPlane, projNearPlane, projFarPlane);
		return self();
	}
};

// 1x1matrices: no projection
template <class T, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixPerspective<T, 1, 1, Order, Layout, Packed> {
public:
	friend Matrix<T, 1, 1, Order, Layout, Packed>;
	using Inherit = impl::Empty<MatrixPerspective>;
};

// 2x2 matrices : no projection
template <class T, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixPerspective<T, 2, 2, Order, Layout, Packed> {
public:
	friend class Matrix<T, 2, 2, Order, Layout, Packed>;
	using Inherit = impl::Empty<MatrixPerspective>;
};

// 3x3 matrices: 2D->1D projection
template <class T, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixPerspective<T, 3, 3, Order, Layout, Packed>
	: private MatrixProjectiveBase<T, 3, Order, Layout, Packed>
{
	using MatrixT = Matrix<T, 3, 3, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	/// <summary> Creates a 2D projection matrix. </summary>
	/// <param name="fov"/> Field of view. </param>
	/// <param name="nearPlane"/> Lower bound of the volume on the Y axis. </param>
	/// <param name="farPlane"/> Upper bound of the volume on the Y axis. </param>
	/// <param name="projNearPlane"/> Near plane is taken here after projection. </param>
	/// <param name="projFarPlane"/> Far plane is taken here after projection. </param>
	/// <remarks> Post-projection bounds may be inverted. </summary>
	static MatrixT Perspective(T fov, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1) {
		return MatrixProjectiveBase<T, 3, Order, Layout, Packed>::Perspective(
			abs(fov), Vector<T, 1, Packed>{ fov < 0 ? -1 : 1 }, nearPlane, farPlane, projNearPlane, projFarPlane);
	}
	/// <summary> Sets this matrix to a 2D projection matrix. </summary>
	/// <param name="fov"/> Field of view. </param>
	/// <param name="nearPlane"/> Lower bound of the volume on the Y axis. </param>
	/// <param name="farPlane"/> Upper bound of the volume on the Y axis. </param>
	/// <param name="projNearPlane"/> Near plane is taken here after projection. </param>
	/// <param name="projFarPlane"/> Far plane is taken here after projection. </param>
	/// <remarks> Post-projection bounds may be inverted. </summary>
	MatrixT& SetPerspective(T fov, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1) {
		self() = Perspective(fov, nearPlane, farPlane, projNearPlane, projFarPlane);
		return self();
	}
public:
	friend MatrixT;
	using Inherit = MatrixPerspective;
};

// 4x4 matrices: 3D->2D projection
template <class T, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixPerspective<T, 4, 4, Order, Layout, Packed>
	: private MatrixProjectiveBase<T, 4, Order, Layout, Packed>
{
	using MatrixT = Matrix<T, 4, 4, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	/// <summary> Creates a 3D projection matrix. </summary>
	/// <param name="fov"/> Field of view. </param>
	/// <param name="aspectRatio"> FovX/FovY, so 1.777 for a 16:9 screen. </param>
	/// <param name="nearPlane"/> Lower bound of the volume on the Y axis. </param>
	/// <param name="farPlane"/> Upper bound of the volume on the Y axis. </param>
	/// <param name="projNearPlane"/> Near plane is taken here after projection. </param>
	/// <param name="projFarPlane"/> Far plane is taken here after projection. </param>
	/// <remarks> Post-projection bounds may be inverted. </summary>
	static MatrixT Perspective(T fov, T aspectRatio, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1) {
		return MatrixProjectiveBase<T, 4, Order, Layout, Packed>::Perspective(
			std::abs(fov), Vector<T, 2, Packed>{ fov < 0 ? -1 : 1, T(1)/aspectRatio}, nearPlane, farPlane, projNearPlane, projFarPlane);
	}
	/// <summary> Sets this matrix to a 3D projection matrix. </summary>
	/// <param name="fov"/> Field of view. </param>
	/// <param name="aspectRatio"> FovX/FovY, so 1.777 for a 16:9 screen. </param>
	/// <param name="nearPlane"/> Lower bound of the volume on the Y axis. </param>
	/// <param name="farPlane"/> Upper bound of the volume on the Y axis. </param>
	/// <param name="projNearPlane"/> Near plane is taken here after projection. </param>
	/// <param name="projFarPlane"/> Far plane is taken here after projection. </param>
	/// <remarks> Post-projection bounds may be inverted. </summary>
	MatrixT& SetPerspective(T fov, T aspectRatio, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1) {
		self() = Perspective(fov, aspectRatio, nearPlane, farPlane, projNearPlane, projFarPlane);
		return self();
	}
public:
	friend MatrixT;
	using Inherit = MatrixPerspective;
};

// Generalized projection matrix
template <class T, int Dim, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixPerspective<T, Dim, Dim, Order, Layout, Packed>
	: public MatrixProjectiveBase<T, Dim, Order, Layout, Packed>
{
	using MatrixT = Matrix<T, Dim, Dim, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	friend MatrixT;
	using Inherit = MatrixPerspective;
};


}