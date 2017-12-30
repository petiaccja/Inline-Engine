//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixOrthographic {
	static constexpr int SpaceDim = Rows != Columns ? std::min(Rows, Columns) : Rows - 1;
	static constexpr bool EnableView = (Rows == Columns
		|| (Order == eMatrixOrder::FOLLOW_VECTOR && Rows - Columns == 1)
		|| (Order == eMatrixOrder::PRECEDE_VECTOR && Columns - Rows == 1))
		&& SpaceDim >= 1;
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	using VectorT = Vector<T, SpaceDim, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	static MatrixT Orthographic(const VectorT& minBounds, const VectorT& maxBounds, T projNearPlane = T(0), T projFarPlane = T(1)) {
		VectorT volumeSize = maxBounds - minBounds;
		VectorT scale = T(2) / volumeSize;
		scale[scale.Dimension() - 1] *= T(0.5)*(projFarPlane - projNearPlane);
		VectorT offset = -(maxBounds + minBounds) / 2 * scale;
		offset[offset.Dimension() - 1] += (projFarPlane + projNearPlane) / 2;

		MatrixT ret;
		ret.SetIdentity();
		for (int i = 0; i < scale.Dimension(); ++i) {
			ret(i, i) = scale(i);
			(Order == eMatrixOrder::FOLLOW_VECTOR ? ret(scale.Dimension(), i) : ret(i, scale.Dimension())) = offset(i);
		}

		return ret;
	}
	MatrixT& SetOrthographic(const VectorT& minBounds, const VectorT& maxBounds, T projNearPlane = T(0), T projFarPlane = T(1)) {
		self() = Orthographic(minBounds, maxBounds, projNearPlane, projFarPlane);
		return self();
	}
public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<EnableView, MatrixOrthographic>;
};


}