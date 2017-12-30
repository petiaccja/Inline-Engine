//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixView {
	static constexpr int SpaceDim = std::max(0, Rows != Columns ? std::min(Rows, Columns) : Rows - 1);
	static constexpr bool EnableView = (Rows == Columns
		|| (Order == eMatrixOrder::FOLLOW_VECTOR && Rows - Columns == 1)
		|| (Order == eMatrixOrder::PRECEDE_VECTOR && Columns - Rows == 1))
		&& SpaceDim >= 2;
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	using VectorT = Vector<T, SpaceDim, Packed>;
public:
	// General function
	static MatrixT LookAt(const VectorT& eye, const VectorT& target, const std::array<VectorT, SpaceDim - 2>& bases, const std::array<bool, SpaceDim>& flipAxes) {
		MatrixT matrix;
		VectorT columns[SpaceDim];
		std::array<const VectorT*, SpaceDim - 1> crossTable = {};
		for (int i = 0; i < (int)bases.size(); ++i) {
			crossTable[i] = &bases[i];
		}
		crossTable.back() = &columns[SpaceDim - 1];
		auto elem = [&matrix](int i, int j) -> T& {
			return Order == eMatrixOrder::FOLLOW_VECTOR ? matrix(i, j) : matrix(j, i);
		};

		// calculate columns of the rotation matrix
		int j = SpaceDim - 1;
		columns[j] = Normalized(eye - target); // right-handed: camera look towards -Z
		do {
			--j;

			columns[SpaceDim - j - 2] = Normalized(Cross(crossTable));

			// shift bases
			for (int s = 0; s < j; ++s) {
				crossTable[s] = crossTable[s + 1];
			}
			crossTable[j] = &columns[SpaceDim - j - 2];
		} while (j > 0);

		// flip columns
		for (int i = 0; i < SpaceDim; ++i) {
			if (flipAxes[i]) {
				columns[i] *= -1;
			}
		}

		// copy columns to matrix
		for (int i = 0; i < SpaceDim; ++i) {
			for (int j = 0; j < SpaceDim; ++j) {
				elem(i, j) = columns[j][i];
			}
		}

		// calculate translation of the matrix
		for (int j = 0; j < SpaceDim; ++j) {
			elem(SpaceDim, j) = -Dot(eye, columns[j]);
		}

		// clear additional elements
		constexpr int AuxDim = Rows < Columns ? Rows : Columns;
		if (AuxDim > SpaceDim) {
			for (int i = 0; i < SpaceDim; ++i) {
				elem(i, AuxDim - 1) = 0;
			}
			elem(SpaceDim, AuxDim - 1) = 1;
		}

		return matrix;
	}

	// Specialization for 2D
	template <class Q = MatrixT>
	static typename std::enable_if<SpaceDim == 2, Q>::type LookAt(const VectorT& eye, const VectorT& target, bool positiveYForward = true, bool flipX = false) {
		return LookAt(eye, target, {}, { flipX, positiveYForward });
	}

	// Specialization for 3D
	template <class Q = MatrixT>
	static typename std::enable_if<SpaceDim == 3, Q>::type LookAt(const VectorT& eye, const VectorT& target, const VectorT& up, bool positiveZForward = true, bool flipX = false, bool flipY = false) {
		return LookAt(eye, target, { up }, { flipX, flipY, positiveZForward });
	}
public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<EnableView, MatrixView>;
};


}