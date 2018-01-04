//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixShear {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
	static constexpr bool Enabled = Rows == Columns;
public:
	static MatrixT Shear(T slope, int principalAxis, int modulatorAxis) {
		assert(principalAxis != modulatorAxis);
		assert(principalAxis < Rows);
		assert(modulatorAxis < Rows);
		MatrixT ret;
		ret.SetIdentity();
		if (Order == eMatrixOrder::FOLLOW_VECTOR) {
			ret(modulatorAxis, principalAxis) = slope;
		}
		else {
			ret(principalAxis, modulatorAxis) = slope;
		}
		return ret;
	}

	MatrixT& SetShear(T slope, int principalAxis, int modulatorAxis) {
		self() = Shear(slope, principalAxis, modulatorAxis); return self();
	}
public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<Enabled, MatrixShear>;
};


}