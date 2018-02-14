//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixScale {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
public:
	/// <summary> Creates a scaling matrix. </summary>
	/// <param name="scales"> A list of scalars corresponding to scaling on respective axes. </summary>
	/// <remarks> The number of arguments must be less than or equal to the matrix dimension. </remarks>
	template <class... Args, typename std::enable_if<(impl::All<impl::IsScalar, typename std::decay<Args>::type...>::value), int>::type = 0>
	static MatrixT Scale(Args&&... scales) {
		static_assert(sizeof...(Args) <= std::min(Rows, Columns), "You must provide scales for dimensions less than or equal to matrix dimension");
		MatrixT m;
		m.SetZero();
		T tableArgs[sizeof...(Args)] = { (T)std::forward<Args>(scales)... };
		int i;
		for (i = 0; i < sizeof...(Args); ++i) {
			m(i, i) = std::move(tableArgs[i]);
		}
		for (; i < std::min(Rows, Columns); ++i) {
			m(i, i) = T(1);
		}
		return m;
	}

	/// <summary> Creates a scaling matrix. </summary>
	/// <param name="scale"> A vector containing the scales of respective axes. </summary>
	/// <remarks> The vector's dimension must be less than or equal to the matrix dimension. </remarks>
	template <class Vt, int Vdim, bool Vpacked>
	static MatrixT Scale(const Vector<Vt, Vdim, Vpacked>& scale) {
		static_assert(Vdim <= std::min(Rows, Columns), "Vector dimension must be smaller than or equal to matrix dimension.");
		MatrixT m;
		m.SetIdentity();
		int i;
		for (i = 0; i < scale.Dimension(); ++i) {
			m(i, i) = std::move(scale(i));
		}
		for (; i < std::min(Rows, Columns); ++i) {
			m(i, i) = T(1);
		}
		return m;
	}

	/// <summary> Set this matrix to a scaling matrix. </summary>
	/// <param name="scales"> A list of scalars corresponding to scaling on respective axes. </summary>
	/// <remarks> The number of arguments must be less than or equal to the matrix dimension. </remarks>
	template <class... Args>
	MatrixT& SetScale(Args&&... args) { self() = Scale(std::forward<Args>(args)...); return self(); }

	/// <summary> Set this matrix to a scaling matrix. </summary>
	/// <param name="scale"> A vector containing the scales of respective axes. </summary>
	/// <remarks> The vector's dimension must be less than or equal to the matrix dimension. </remarks>
	template <class Vt, int Vdim, bool Vpacked>
	MatrixT& SetScale(const Vector<Vt, Vdim, Vpacked>& translation) { self() = Scale(translation); return self(); }
public:
	friend MatrixT;
	using Inherit = MatrixScale;
};


}