//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "MatrixModule.hpp"


namespace mathter {


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class MatrixTranslation {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	MatrixT& self() { return *static_cast<MatrixT*>(this); }
	const MatrixT& self() const { return *static_cast<const MatrixT*>(this); }
protected:
	static constexpr int TranslationDim = Rows == Columns ? Rows - 1 : std::min(Rows, Columns);
	static constexpr bool EnableTranslation =
		TranslationDim > 0 &&
		((Order == eMatrixOrder::FOLLOW_VECTOR && Rows - 1 <= Columns && Columns <= Rows)
			|| (Order == eMatrixOrder::PRECEDE_VECTOR && Columns - 1 <= Rows && Rows <= Columns));
public:
	/// <summary> Creates a translation matrix. </summary>
	/// <param name="translations"> A list of scalars that specify movement along repsective axes. </param>
	template <class... Args, typename std::enable_if<(impl::All<impl::IsScalar, typename std::decay<Args>::type...>::value), int>::type = 0>
	static MatrixT Translation(Args&&... translations) {
		static_assert(sizeof...(Args) == TranslationDim, "Number of arguments must match the dimension of translation.");

		MatrixT m;
		m.SetIdentity();
		T tableArgs[sizeof...(Args)] = { (T)std::forward<Args>(translations)... };
		if (Order == eMatrixOrder::FOLLOW_VECTOR) {
			for (int i = 0; i < sizeof...(Args); ++i) {
				m(Rows - 1, i) = std::move(tableArgs[i]);
			}
		}
		else {
			for (int i = 0; i < sizeof...(Args); ++i) {
				m(Columns - 1, i) = std::move(tableArgs[i]);
			}
		}
		return m;
	}

	/// <summary> Creates a translation matrix. </summary>
	/// <param name="translation"> The movement vector. </param>
	template <class Vt, bool Vpacked>
	static MatrixT Translation(const Vector<Vt, TranslationDim, Vpacked>& translation) {
		MatrixT m;
		m.SetIdentity();
		if (Order == eMatrixOrder::FOLLOW_VECTOR) {
			for (int i = 0; i < translation.Dimension(); ++i) {
				m(Rows - 1, i) = translation(i);
			}
		}
		else {
			for (int i = 0; i < translation.Dimension(); ++i) {
				m(i, Columns - 1) = translation(i);
			}
		}
		return m;
	}

	/// <summary> Sets this matrix to a translation matrix. </summary>
	/// <param name="translations"> A list of scalars that specify movement along repsective axes. </param>
	template <class... Args>
	MatrixT& SetTranslation(Args&&... args) { self() = Translation(std::forward<Args>(args)...); return self(); }

	/// <summary> Sets this matrix to a translation matrix. </summary>
	/// <param name="translation"> The movement vector. </param>
	template <class Vt, bool Vpacked>
	MatrixT& SetTranslation(const Vector<Vt, TranslationDim, Vpacked>& translation) { self() = Translation(translation); return self(); }
public:
	friend MatrixT;
	using Inherit = impl::MatrixModule<EnableTranslation, MatrixTranslation>;
};


}