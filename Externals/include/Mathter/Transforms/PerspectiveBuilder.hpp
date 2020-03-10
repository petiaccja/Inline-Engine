#pragma once


#include "../Matrix/MatrixImpl.hpp"
#include "../Vector.hpp"
#include "IdentityBuilder.hpp"

namespace mathter {


template <class T, int Dim, bool Packed>
class PerspectiveBuilder {
	static_assert(!std::is_integral_v<T>);

public:
	PerspectiveBuilder(T fovX, const Vector<T, Dim - 1, Packed>& ratios, T nearPlane, T farPlane, T projNearPlane = 0, T projFarPlane = 1)
		: fovX(fovX), ratios(ratios), nearPlane(nearPlane), farPlane(farPlane), projNearPlane(projNearPlane), projFarPlane(projFarPlane) {}
	PerspectiveBuilder& operator=(const PerspectiveBuilder&) = delete;

	template <class U, eMatrixOrder Order, eMatrixLayout Layout, bool MPacked>
	operator Matrix<U, Dim + 1, Dim + 1, Order, Layout, MPacked>() const {
		Matrix<U, Dim + 1, Dim + 1, Order, Layout, MPacked> m;
		Set(m);
		return m;
	}

private:
	template <class U, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool MPacked>
	void Set(Matrix<U, Rows, Columns, Order, Layout, MPacked>& m) const {
		assert((nearPlane < 0 && farPlane < nearPlane) || (0 < nearPlane && nearPlane < farPlane));

		m = Zero();
		// Layout be like (precede_vector):
		// w 0 0 0
		// 0 h 0 0
		// 0 0 A B
		// 0 0 C 0
		auto absFovX = std::abs(fovX);
		T N = nearPlane;
		T F = farPlane;
		T n = projNearPlane;
		T f = projFarPlane;
		T C = nearPlane < T(0) ? T(-1) : T(1);
		T A = C * (f * F - n * N) / (F - N);
		T B = C * F * N * (n - f) / (F - N);
		Vector<T, Dim - 1, Packed> adjRatios = ratios(0) / ratios;
		T w = tan(T(0.5) * absFovX);
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
	}

	const T fovX;
	const Vector<T, Dim - 1, Packed> ratios;
	const T nearPlane;
	const T farPlane;
	const T projNearPlane = 0;
	const T projFarPlane = 1;
};


/// <summary> Creates a perspective projection matrix. </summary>
/// <param name="fovX"> Field of view on the first axis (usually denoted X) in radians. </param>
/// <param name="ratios"> Aspect ratio (or ratios in higher dimensions). FovX/FovY. </param>
/// <param name="nearPlane"> Near bound of the projected volume on the last axis (Z in 3D). </param>
/// <param name="farPlane"> Far bound of the projected volume on the last axis (Z in 3D). </param>
/// <param name="projNearPlane"> The near plane is taken here after projection. </param>
/// <param name="projFarPlane"> The far plane is taken here after projection. </param>
/// <remarks> Post-projection near and far planes can be inverted. Negative ratios invert image. </remarks>
template <class T, int DimMinus1, bool Packed>
auto Perspective(T fovX, const Vector<T, DimMinus1, Packed>& ratios, T nearPlane, T farPlane, T projNearPlane = T(0), T projFarPlane = T(1)) {
	using NonIntegral = std::conditional_t<std::is_integral_v<T>, float, T>;
	return PerspectiveBuilder<NonIntegral, DimMinus1 + 1, Packed>{ fovX, ratios, nearPlane, farPlane, projNearPlane, projFarPlane };
}


/// <summary> Creates a 2D projection matrix. </summary>
/// <param name="fov"> Field of view. </param>
/// <param name="nearPlane"> Lower bound of the volume on the Y axis. </param>
/// <param name="farPlane"> Upper bound of the volume on the Y axis. </param>
/// <param name="projNearPlane"> Near plane is taken here after projection. </param>
/// <param name="projFarPlane"> Far plane is taken here after projection. </param>
/// <remarks> Post-projection bounds may be inverted. </remarks>
template <class T>
auto Perspective(T fov, T nearPlane, T farPlane, T projNearPlane = T(0), T projFarPlane = T(1)) {
	return Perspective(std::abs(fov), Vector<T, 1, false>{ fov < 0 ? -1 : 1 }, nearPlane, farPlane, projNearPlane, projFarPlane);
}


/// <summary> Creates a 3D projection matrix. </summary>
/// <param name="fov"/> Field of view. </param>
/// <param name="aspectRatio"> FovX/FovY, so 1.777 for a 16:9 screen. </param>
/// <param name="nearPlane"/> Lower bound of the volume on the Y axis. </param>
/// <param name="farPlane"/> Upper bound of the volume on the Y axis. </param>
/// <param name="projNearPlane"/> Near plane is taken here after projection. </param>
/// <param name="projFarPlane"/> Far plane is taken here after projection. </param>
/// <remarks> Post-projection bounds may be inverted. </summary>
template <class T>
auto Perspective(T fov, T aspectRatio, T nearPlane, T farPlane, T projNearPlane = T(0), T projFarPlane = T(1)) {
	return Perspective(std::abs(fov), Vector<T, 2, false>{ fov < 0 ? -1 : 1, T(1) / aspectRatio }, nearPlane, farPlane, projNearPlane, projFarPlane);
}



} // namespace mathter