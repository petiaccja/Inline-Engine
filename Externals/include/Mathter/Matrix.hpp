//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

// Remove goddamn fucking bullshit crapware winapi macros.
#if _MSC_VER && defined(min)
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#define MATHTER_MINMAX
#endif


#include <type_traits>
#include <iostream> // debug only
#include <algorithm>
#include <array>

#include "Vector.hpp"
#include "DefinitionsUtil.hpp"


#include "MatrixModules/DecomposeLU.hpp"
#include "MatrixModules/DecomposeQR.hpp"
#include "MatrixModules/DecomposeSVD.hpp"
#include "MatrixModules/Orthographic.hpp"
#include "MatrixModules/Perspective.hpp"
#include "MatrixModules/Rotation2D.hpp"
#include "MatrixModules/Rotation3D.hpp"
#include "MatrixModules/Scale.hpp"
#include "MatrixModules/Shear.hpp"
#include "MatrixModules/Square.hpp"
#include "MatrixModules/Translation.hpp"
#include "MatrixModules/View.hpp"


namespace mathter {


//------------------------------------------------------------------------------
// Matrix base class only allocating the memory
//------------------------------------------------------------------------------

template <class T, int Rows, int Columns, eMatrixOrder Order = eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout Layout = eMatrixLayout::ROW_MAJOR, bool Packed = false>
class MatrixData {
public:
	constexpr int ColumnCount() const {
		return Columns;
	}
	constexpr int RowCount() const {
		return Rows;
	}
	constexpr int Width() const {
		return Columns;
	}
	constexpr int Height() const {
		return Rows;
	}
protected:
	// Rows equal height, Columns equal width, row-major has column-sized stripes
	static constexpr int StripeDim = Layout == eMatrixLayout::ROW_MAJOR ? Columns : Rows;
	static constexpr int StripeCount = Layout == eMatrixLayout::ROW_MAJOR ? Rows : Columns;

	std::array<Vector<T, StripeDim, Packed>, StripeCount> stripes;

	// Get element
	inline T& GetElement(int row, int col) {
		assert(row < RowCount());
		assert(col < ColumnCount());
		return GetElementImpl(col, row, std::integral_constant<bool, Layout == eMatrixLayout::ROW_MAJOR>());
	}
	inline T GetElement(int row, int col) const {
		assert(row < RowCount());
		assert(col < ColumnCount());
		return GetElementImpl(col, row, std::integral_constant<bool, Layout == eMatrixLayout::ROW_MAJOR>());
	}
private:
	inline T& GetElementImpl(int col, int row, std::true_type) {
		return stripes[row][col];
	}
	inline T GetElementImpl(int col, int row, std::true_type) const {
		return stripes[row][col];
	}
	inline T& GetElementImpl(int col, int row, std::false_type) {
		return stripes[col][row];
	}
	inline T GetElementImpl(int col, int row, std::false_type) const {
		return stripes[col][row];
	}
};


//------------------------------------------------------------------------------
// Submatrix helper
//------------------------------------------------------------------------------

template <class MatrixT, int SRows, int SColumns>
class SubmatrixHelper {
	friend MatrixT;
	using Props = impl::MatrixProperties<MatrixT>;
	template <class, int, int>
	friend class SubmatrixHelper;
	static constexpr int VecDim = std::max(SRows, SColumns);
	static constexpr bool VectorAssignable = std::min(SRows, SColumns) == 1;
protected:
	SubmatrixHelper(MatrixT& mat, int row, int col) : mat(mat), row(row), col(col) {}

public:
	SubmatrixHelper(const SubmatrixHelper& rhs) = delete;
	SubmatrixHelper(SubmatrixHelper&& rhs) : mat(rhs.mat), row(rhs.row), col(rhs.col) {}


	template <class U, eMatrixOrder UOrder, eMatrixLayout ULayout, bool UPacked>
	operator Matrix<U, SRows, SColumns, UOrder, ULayout, UPacked>() const {
		Matrix<U, SRows, SColumns, UOrder, ULayout, UPacked> ret;
		for (int i = 0; i < SRows; ++i) {
			for (int j = 0; j < SColumns; ++j) {
				ret(i, j) = (*this)(i, j);
			}
		}
		return ret;
	}

	template <class U, bool Packed2, class = typename std::enable_if<VectorAssignable, U>::type>
	operator Vector<U, VecDim, Packed2>() const {
		Vector<U, std::max(SRows, SColumns), Packed2> v;
		int k = 0;
		for (int i = 0; i < SRows; ++i) {
			for (int j = 0; j < SColumns; ++j) {
				v(k) = (*this)(i, j);
				++k;
			}
		}
		return v;
	}


	template <class U, eMatrixOrder UOrder, eMatrixLayout ULayout, bool UPacked>
	SubmatrixHelper& operator=(const Matrix<U, SRows, SColumns, UOrder, ULayout, UPacked>& rhs) {
		static_assert(!std::is_const<MatrixT>::value, "Cannot assign to submatrix of const matrix.");

		// If aliasing happens, the same matrix is copied to itself with no side-effects.
		for (int i = 0; i < SRows; ++i) {
			for (int j = 0; j < SColumns; ++j) {
				mat(row + i, col + j) = rhs(i, j);
			}
		}
		return *this;
	}


	// From vector if applicable (for 1*N and N*1 submatrices)
	template <class U, bool Packed, class = typename std::enable_if<VectorAssignable, U>::type>
	SubmatrixHelper& operator=(const Vector<U, VecDim, Packed>& v) {
		static_assert(!std::is_const<MatrixT>::value, "Cannot assign to submatrix of const matrix.");

		int k = 0;
		for (int i = 0; i < SRows; ++i) {
			for (int j = 0; j < SColumns; ++j) {
				mat(row + i, col + j) = v(k);
				++k;
			}
		}
		return *this;
	}


	template <class MatrixU>
	SubmatrixHelper& operator=(const SubmatrixHelper<MatrixU, SRows, SColumns>& rhs) {
		static_assert(!std::is_const<MatrixT>::value, "Cannot assign to submatrix of const matrix.");

		// If *this and rhs reference the same matrix, aliasing must be resolved.
		if ((void*)&mat == (void*)&rhs.mat) {
			Matrix<typename impl::MatrixProperties<MatrixU>::Type,
					SRows,
					SColumns,
					impl::MatrixProperties<MatrixU>::Order,
					impl::MatrixProperties<MatrixU>::Layout,
					impl::MatrixProperties<MatrixU>::Packed> tmpmat;
			tmpmat = rhs;
			operator=(tmpmat);
		}
		else {
			for (int i = 0; i < SRows; ++i) {
				for (int j = 0; j < SColumns; ++j) {
					mat(row + i, col + j) = rhs(i, j);
				}
			}
		}
		return *this;
	}
	SubmatrixHelper& operator=(const SubmatrixHelper& rhs) {
		static_assert(!std::is_const<MatrixT>::value, "Cannot assign to submatrix of const matrix.");
		return operator=<MatrixT>(rhs);
	}

	typename Props::Type& operator()(int row, int col) {
		return mat(this->row + row, this->col + col);
	}

	typename Props::Type operator()(int row, int col) const {
		return mat(this->row + row, this->col + col);
	}
private:
	MatrixT& mat;
	int row = -1, col = -1;
};


//------------------------------------------------------------------------------
// Global Matrix function prototypes
//------------------------------------------------------------------------------

// Same layout
template <
	class T,
	class U,
	int Rows,
	int Columns,
	eMatrixOrder Order1,
	eMatrixOrder Order2,
	eMatrixLayout SameLayout,
	bool Packed,
	class V = decltype(T() + U())
>
Matrix<U, Rows, Columns, Order1, SameLayout, Packed> operator+(
	const Matrix<T, Rows, Columns, Order1, SameLayout, Packed>&,
	const Matrix<U, Rows, Columns, Order2, SameLayout, Packed>&);


template <
	class T,
	class U,
	int Rows,
	int Columns,
	eMatrixOrder Order1,
	eMatrixOrder Order2,
	eMatrixLayout SameLayout,
	bool Packed,
	class V = decltype(T() - U())
>
Matrix<U, Rows, Columns, Order1, SameLayout, Packed> operator-(
	const Matrix<T, Rows, Columns, Order1, SameLayout, Packed>&,
	const Matrix<U, Rows, Columns, Order2, SameLayout, Packed>&);


// Opposite layout
template <
	class T,
	class U,
	int Rows,
	int Columns,
	eMatrixOrder Order1,
	eMatrixOrder Order2,
	eMatrixLayout Layout1,
	eMatrixLayout Layout2,
	bool Packed,
	class V = decltype(T() + U()),
	class = typename std::enable_if<Layout1 != Layout2>::type
>
Matrix<U, Rows, Columns, Order1, Layout1, Packed> operator+(
	const Matrix<T, Rows, Columns, Order1, Layout1, Packed>&,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>&);

template <
	class T,
	class U,
	int Rows,
	int Columns,
	eMatrixOrder Order1,
	eMatrixOrder Order2,
	eMatrixLayout Layout1,
	eMatrixLayout Layout2,
	bool Packed,
	class V = decltype(T() - U()),
	class = typename std::enable_if<Layout1 != Layout2>::type
>
Matrix<U, Rows, Columns, Order1, Layout1, Packed> operator-(
	const Matrix<T, Rows, Columns, Order1, Layout1, Packed>&,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>&);



//------------------------------------------------------------------------------
// Matrix class providing the common interface for all matrices
//------------------------------------------------------------------------------

template <class T, int Rows, int Columns, eMatrixOrder Order = eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout Layout = eMatrixLayout::ROW_MAJOR, bool Packed = false>
class MATHTER_EBCO Matrix
	: public MatrixData<T, Rows, Columns, Order, Layout, Packed>,
	public MatrixLU<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixQR<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixSVD<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixOrthographic<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixPerspective<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixRotation2D<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixScale<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixShear<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixSquare<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixTranslation<T, Rows, Columns, Order, Layout, Packed>::Inherit,
	public MatrixView<T, Rows, Columns, Order, Layout, Packed>::Inherit
{
	static_assert(Columns >= 1 && Rows >= 1, "Dimensions must be positive integers.");
	// Make a call to this function in EVERY constructor of the Matrix class.
	// These checks must be put in a separate function instead of class scope because the full definition
	// of the Matrix class is required to determine memory layout.
	void CheckLayoutContraints() const noexcept;

	static constexpr int VecDim = std::max(Rows, Columns);
	static constexpr bool VectorAssignable = std::min(Rows, Columns) == 1;
protected:
	using MatrixData<T, Rows, Columns, Order, Layout, Packed>::GetElement;
	using MatrixData<T, Rows, Columns, Order, Layout, Packed>::stripes;
	using MatrixData<T, Rows, Columns, Order, Layout, Packed>::StripeCount;

	template <class T2, int Dim, eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	friend class mathter::DecompositionLU;
	template <class T2, int Rows2, int Columns2, eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	friend class mathter::MatrixSVD;

	template <class T2, int Rows2, int Columns2, eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	friend class Matrix;
public:
	static void DumpLayout(std::ostream& os) {
		Matrix* ptr = reinterpret_cast<Matrix*>(1000);
		using T1 = MatrixData<T, Rows, Columns, Order, Layout, Packed>;
		using T2 = typename MatrixSquare<T, Rows, Columns, Order, Layout, Packed>::Inherit;
		using T3 = typename MatrixRotation2D<T, Rows, Columns, Order, Layout, Packed>::Inherit;
		using T4 = typename MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::Inherit;
		using T5 = typename MatrixTranslation<T, Rows, Columns, Order, Layout, Packed>::Inherit;
		using T6 = typename MatrixScale<T, Rows, Columns, Order, Layout, Packed>::Inherit;
		os << "MatrixData:        " << (intptr_t)static_cast<T1*>(ptr) - 1000 << " -> " << sizeof(T1) << std::endl;
		os << "MatrixSquare:      " << (intptr_t)static_cast<T2*>(ptr) - 1000 << " -> " << sizeof(T2) << std::endl;
		os << "MatrixRotation2D:  " << (intptr_t)static_cast<T3*>(ptr) - 1000 << " -> " << sizeof(T3) << std::endl;
		os << "MatrixRotation3D:  " << (intptr_t)static_cast<T4*>(ptr) - 1000 << " -> " << sizeof(T4) << std::endl;
		os << "MatrixTranslation: " << (intptr_t)static_cast<T5*>(ptr) - 1000 << " -> " << sizeof(T5) << std::endl;
		os << "MatrixScale:       " << (intptr_t)static_cast<T6*>(ptr) - 1000 << " -> " << sizeof(T6) << std::endl;
	}

	using MatrixData<T, Rows, Columns, Order, Layout, Packed>::RowCount;
	using MatrixData<T, Rows, Columns, Order, Layout, Packed>::ColumnCount;

	//--------------------------------------------
	// Constructors
	//--------------------------------------------

	Matrix() {
		CheckLayoutContraints();
	}
	
	// From same multiplication order
	template <class T2, eMatrixLayout Layout2, bool Packed2>
	Matrix(const Matrix<T2, Rows, Columns, Order, Layout2, Packed2>& rhs) {
		CheckLayoutContraints();
		for (int i = 0; i < RowCount(); ++i) {
			for (int j = 0; j < ColumnCount(); ++j) {
				(*this)(i, j) = rhs(i, j);
			}
		}
	}

	// From opposite multiplication order
	template <class T2, eMatrixLayout Layout2, bool Packed2>
	Matrix(const Matrix<T2, Columns, Rows, Order == eMatrixOrder::FOLLOW_VECTOR ? eMatrixOrder::PRECEDE_VECTOR : eMatrixOrder::FOLLOW_VECTOR, Layout2, Packed2>& rhs) {
		CheckLayoutContraints();
		for (int i = 0; i < RowCount(); ++i) {
			for (int j = 0; j < ColumnCount(); ++j) {
				(*this)(i, j) = rhs(j, i); // Transpose argument
			}
		}
	}

	template <class H, class... Args, typename std::enable_if<impl::All<impl::IsScalar, H, Args...>::value, int>::type = 0>
	Matrix(H h, Args... args) {
		CheckLayoutContraints();

		static_assert(1 + sizeof...(Args) == Columns*Rows, "All elements of matrix have to be initialized.");
		Assign<0, 0>(h, args...);
	}

	// From vector if applicable (for 1*N and N*1 matrices)
	template <class T2, bool Packed2, class = typename std::enable_if<VectorAssignable, T2>::type>
	Matrix(const Vector<T2, VecDim, Packed2>& v){
		for (int i = 0; i < v.Dimension(); ++i) {
			(*this)(i) = v(i);
		}
	}


	//--------------------------------------------
	// Accessors
	//--------------------------------------------

	// General matrix indexing
	inline T& operator()(int row, int col) {
		return GetElement(row, col);
	}
	inline T operator()(int row, int col) const {
		return GetElement(row, col);
	}

	// Column and row vector simple indexing
	template <class Q = T>
	inline typename std::enable_if<(Columns == 1 && Rows > 1) || (Columns > 1 && Rows == 1), Q>::type& operator()(int idx) {
		return GetElement(Rows == 1 ? 0 : idx, Columns == 1 ? 0 : idx);
	}
	template <class Q = T>
	inline typename std::enable_if<(Columns == 1 && Rows > 1) || (Columns > 1 && Rows == 1), Q>::type operator()(int idx) const {
		return GetElement(Rows == 1 ? 0 : idx, Columns == 1 ? 0 : idx);
	}

	// Submatrices
	template <int Subrows, int Subcolumns> 
	mathter::SubmatrixHelper<Matrix, Subrows, Subcolumns> Submatrix(int rowIdx, int colIdx) {
		assert(Subrows + rowIdx <= Rows);
		assert(Subcolumns + colIdx <= Columns);

		return SubmatrixHelper<Matrix, Subrows, Subcolumns>(*this, rowIdx, colIdx);
	}

	template <int Subrows, int Subcolumns>
	mathter::SubmatrixHelper<const Matrix, Subrows, Subcolumns> Submatrix(int rowIdx, int colIdx) const {
		assert(Subrows + rowIdx <= Rows);
		assert(Subcolumns + colIdx <= Columns);

		return SubmatrixHelper<const Matrix, Subrows, Subcolumns>(*this, rowIdx, colIdx);
	}

	auto Column(int colIdx) {
		return Submatrix<Rows, 1>(0, colIdx);
	}
	auto Row(int rowIdx) {
		return Submatrix<1, Columns>(rowIdx, 0);
	}
	auto Column(int colIdx) const {
		return Submatrix<Rows, 1>(0, colIdx);
	}
	auto Row(int rowIdx) const {
		return Submatrix<1, Columns>(rowIdx, 0);
	}

	// Conversion to vector if applicable
	template <class T2, bool Packed2, class = typename std::enable_if<VectorAssignable, T2>::type>
	operator Vector<T2, VecDim, Packed2>() const {
		Vector<T2, std::max(Rows, Columns), Packed2> v;
		int k = 0;
		for (int i = 0; i < Rows; ++i) {
			for (int j = 0; j < Columns; ++j) {
				v(k) = (*this)(i, j);
				++k;
			}
		}
		return v;
	}

	//--------------------------------------------
	// Compare
	//--------------------------------------------

	template <eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	bool operator==(const Matrix<T, Rows, Columns, Order2, Layout2, Packed2>& rhs) const {
		bool equal = true;
		for (int i = 0; i < StripeCount; ++i) {
			equal = equal && stripes[i] == rhs.stripes[i];
		}
		return equal;
	}

	template <eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	bool operator!=(const Matrix<T, Rows, Columns, Order2, Layout2, Packed2>& rhs) const {
		return !(*this == rhs);
	}

	template <eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	bool AlmostEqual(const Matrix<T, Rows, Columns, Order2, Layout2, Packed2>& rhs, std::true_type) const {
		bool equal = true;
		for (int i = 0; i < RowCount(); ++i) {
			for (int j = 0; j < ColumnCount(); ++j) {
				equal = equal && impl::AlmostEqual((*this)(i, j), rhs(i, j));
			}
		}
		return equal;
	}
	template <eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	bool AlmostEqual(const Matrix<T, Rows, Columns, Order2, Layout2, Packed2>& rhs, std::false_type) const {
		return *this == rhs;
	};
	template <eMatrixOrder Order2, eMatrixLayout Layout2, bool Packed2>
	bool AlmostEqual(const Matrix<T, Rows, Columns, Order2, Layout2, Packed2>& rhs) const {
		return AlmostEqual(rhs, std::integral_constant<bool, std::is_floating_point<T>::value>());
	};

	auto Approx() const {
		return mathter::ApproxHelper<Matrix>(*this);
	}

	//--------------------------------------------
	// Arithmetic
	//--------------------------------------------

	// Non-modifying external operators

	// Same layout
	template <class T1, class T2, int RowsA, int ColumnsA, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout SameLayout, bool PackedA, class V>
	inline friend Matrix<T2, RowsA, ColumnsA, Order1, SameLayout, PackedA> operator+(
		const Matrix<T1, RowsA, ColumnsA, Order1, SameLayout, PackedA>&,
		const Matrix<T2, RowsA, ColumnsA, Order2, SameLayout, PackedA>&);

	template <class T1, class T2, int RowsA, int ColumnsA, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout SameLayout, bool PackedA, class V>
	inline friend Matrix<T2, RowsA, ColumnsA, Order1, SameLayout, PackedA> operator-(
		const Matrix<T1, RowsA, ColumnsA, Order1, SameLayout, PackedA>&,
		const Matrix<T2, RowsA, ColumnsA, Order2, SameLayout, PackedA>&);
	

	// Multiplication row-row
	template <class T1, class T2, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool PackedA, class V>
	inline friend auto operator*(const Matrix<T1, Rows1, Match, Order1, eMatrixLayout::ROW_MAJOR, PackedA>& lhs,
						  const Matrix<T2, Match, Columns2, Order2, eMatrixLayout::ROW_MAJOR, PackedA>& rhs)
		->Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, PackedA>;

	// Multiplication row-col
	template <class T1, class T2, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool PackedA, class V>
	inline friend auto operator*(const Matrix<T1, Rows1, Match, Order1, eMatrixLayout::ROW_MAJOR, PackedA>& lhs,
						  const Matrix<T2, Match, Columns2, Order2, eMatrixLayout::COLUMN_MAJOR, PackedA>& rhs)
		->Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, PackedA>;

	// Multiplication col-col
	template <class T1, class T2, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool PackedA, class V>
	inline friend auto operator*(const Matrix<T1, Rows1, Match, Order1, eMatrixLayout::COLUMN_MAJOR, PackedA>& lhs,
				   const Matrix<T2, Match, Columns2, Order2, eMatrixLayout::COLUMN_MAJOR, PackedA>& rhs)
		->Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, PackedA>;

	// Multiplication col-row
	template <class T1, class T2, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool PackedA, class V>
	inline friend auto operator*(const Matrix<T1, Rows1, Match, Order1, eMatrixLayout::COLUMN_MAJOR, PackedA>& lhs,
						  const Matrix<T2, Match, Columns2, Order2, eMatrixLayout::ROW_MAJOR, PackedA>& rhs)
		->Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, PackedA>;


	// Scalar multiplication
	inline Matrix operator*(T s) const {
		return Matrix(*this) *= s;
	}

	inline Matrix operator/(T s) const {
		return Matrix(*this) /= s;
	}

	// Unary signs
	inline Matrix operator+() const {
		return Matrix(*this);
	}

	inline Matrix operator-() const {
		return Matrix(*this) *= T(-1);
	}


	// Internal modifying operators

	// Addition, subtraction
	template <class U, eMatrixOrder Order2, eMatrixLayout Layout2>
	inline Matrix& operator+=(const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs) {
		*this = mathter::operator+<T, U, Rows, Columns, Order, Order2, Layout2, Packed, T>(*this, rhs);
		return *this;
	}

	template <class U, eMatrixOrder Order2, eMatrixLayout Layout2>
	inline Matrix& operator-=(const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs) {
		*this = mathter::operator-<T, U, Rows, Columns, Order, Order2, Layout2, Packed, T>(*this, rhs);
		return *this;
	}

	// Scalar multiplication
	inline Matrix& operator*=(T s) {
		for (auto& stripe : stripes) {
			stripe *= s;
		}
		return *this;
	}
	inline Matrix& operator/=(T s) {
		*this *= (1 / s);
		return *this;
	}


	// Elementwise multiply and divide
	template <class T2, eMatrixOrder Order2>
	inline Matrix& MulElementwise(const Matrix<T2, Rows, Columns, Order2, Layout, Packed>& rhs) {
		for (int i = 0; i < StripeCount; ++i) {
			stripes[i] = stripes[i] * rhs.stripes[i];
		}
		return *this;
	}

	template <class T2, eMatrixOrder Order2>
	inline Matrix& MulElementwise(const Matrix<T2, Rows, Columns, Order2, impl::OppositeLayout<Layout>::value, Packed>& rhs) {
		for (int i = 0; i < RowCount(); ++i) {
			for (int j = 0; j < ColumnCount(); ++j) {
				(*this)(i, j) *= rhs(i, j);
			}
		}
		return *this;
	}

	template <class T2, eMatrixOrder Order2>
	inline Matrix& DivElementwise(const Matrix<T2, Rows, Columns, Order2, Layout, Packed>& rhs) {
		for (int i = 0; i < StripeCount; ++i) {
			stripes[i] = stripes[i] / rhs.stripes[i];
		}
		return *this;
	}

	template <class T2, eMatrixOrder Order2>
	inline Matrix& DivElementwise(const Matrix<T2, Rows, Columns, Order2, impl::OppositeLayout<Layout>::value, Packed>& rhs) {
		for (int i = 0; i < RowCount(); ++i) {
			for (int j = 0; j < ColumnCount(); ++j) {
				(*this)(i, j) /= rhs(i, j);
			}
		}
		return *this;
	}


	//--------------------------------------------
	// Matrix functions
	//--------------------------------------------
	auto Transposed() const -> Matrix<T, Columns, Rows, Order, Layout, Packed> {
		Matrix<T, Columns, Rows, Order, Layout, Packed> result;
		for (int i = 0; i < RowCount(); ++i) {
			for (int j = 0; j < ColumnCount(); ++j) {
				result(j,i) = (*this)(i,j);
			}
		}
		return result;
	}

	static Matrix Zero() {
		Matrix m;
		for (auto& stripe : m.stripes) {
			stripe.Spread(T(0));
		}
		return m;
	}

	Matrix& SetZero() {
		*this = Zero();
		return *this;
	}

	static Matrix Identity();
	Matrix& SetIdentity();

	T Norm() const {
		return sqrt(NormSquared());
	}
	T NormSquared() const {
		T sum = T(0);
		for (auto& stripe : stripes) {
			sum += stripe.LengthSquared();
		}
		sum /= (RowCount() * ColumnCount());
		return sum;
	}

	//--------------------------------------------
	// Matrix-vector arithmetic
	//--------------------------------------------
	template <class Vt, class Mt, int Vd, int Mcol, eMatrixOrder Morder, bool PackedA, class Rt>
	inline friend Vector<Rt, Mcol, PackedA> operator*(const Vector<Vt, Vd, PackedA>& vec, const Matrix<Mt, Vd, Mcol, Morder, eMatrixLayout::ROW_MAJOR, PackedA>& mat);

	template <class Vt, class Mt, int Vd, int Mrow, eMatrixOrder Morder, bool PackedA, class Rt>
	inline friend Vector<Rt, Mrow, PackedA> operator*(const Matrix<Mt, Mrow, Vd, Morder, eMatrixLayout::ROW_MAJOR, PackedA>& mat, const Vector<Vt, Vd, PackedA>& vec);


protected:
	//--------------------------------------------
	// Helpers
	//--------------------------------------------

	template <int i, int j, class Head, class... Args>
	void Assign(Head head, Args... args) { 
		(*this)(i, j) = (T)head;
		Assign<((j != Columns - 1) ? i : (i + 1)), ((j + 1) % Columns)>(args...);
	}

	template <int, int>
	void Assign() {}
};



//------------------------------------------------------------------------------
// Matrix-Matrix arithmetic
//------------------------------------------------------------------------------

// Macros for manual matrix multiplication loop unrolling

// Row-major * Row-major
#define MATHTER_MATMUL_EXPAND(...) __VA_ARGS__

#define MATHTER_MATMUL_RR_FACTOR(X, Y) rhs.stripes[X] * lhs(Y, X)

#define MATHTER_MATMUL_RR_STRIPE_1(Y) MATHTER_MATMUL_RR_FACTOR(0, Y)
#define MATHTER_MATMUL_RR_STRIPE_2(Y) MATHTER_MATMUL_RR_STRIPE_1(Y) + MATHTER_MATMUL_RR_FACTOR(1, Y)
#define MATHTER_MATMUL_RR_STRIPE_3(Y) MATHTER_MATMUL_RR_STRIPE_2(Y) + MATHTER_MATMUL_RR_FACTOR(2, Y)
#define MATHTER_MATMUL_RR_STRIPE_4(Y) MATHTER_MATMUL_RR_STRIPE_3(Y) + MATHTER_MATMUL_RR_FACTOR(3, Y)
#define MATHTER_MATMUL_RR_STRIPE(CX, Y) MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_RR_STRIPE_ ## CX)(Y)

#define MATHTER_MATMUL_RR_ARRAY_1(CX) result.stripes[0] = MATHTER_MATMUL_RR_STRIPE(CX, 0) ;
#define MATHTER_MATMUL_RR_ARRAY_2(CX) MATHTER_MATMUL_RR_ARRAY_1(CX) result.stripes[1] = MATHTER_MATMUL_RR_STRIPE(CX, 1) ;
#define MATHTER_MATMUL_RR_ARRAY_3(CX) MATHTER_MATMUL_RR_ARRAY_2(CX) result.stripes[2] = MATHTER_MATMUL_RR_STRIPE(CX, 2) ;
#define MATHTER_MATMUL_RR_ARRAY_4(CX) MATHTER_MATMUL_RR_ARRAY_3(CX) result.stripes[3] = MATHTER_MATMUL_RR_STRIPE(CX, 3) ;

#define MATHTER_MATMUL_RR_ARRAY(CX, CY) MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_RR_ARRAY_ ## CY)(CX)

#define MATHTER_MATMUL_RR_UNROLL(MATCH, ROWS1) if (Rows1 == ROWS1 && Match == MATCH ) { MATHTER_MATMUL_RR_ARRAY(MATCH, ROWS1) return result; }

// Column-major * Column-major
#define MATHTER_MATMUL_CC_FACTOR(X, Y) lhs.stripes[Y] * rhs(Y, X)

#define MATHTER_MATMUL_CC_STRIPE_1(X) MATHTER_MATMUL_CC_FACTOR(X, 0)
#define MATHTER_MATMUL_CC_STRIPE_2(X) MATHTER_MATMUL_CC_STRIPE_1(X) + MATHTER_MATMUL_CC_FACTOR(X, 1)
#define MATHTER_MATMUL_CC_STRIPE_3(X) MATHTER_MATMUL_CC_STRIPE_2(X) + MATHTER_MATMUL_CC_FACTOR(X, 2)
#define MATHTER_MATMUL_CC_STRIPE_4(X) MATHTER_MATMUL_CC_STRIPE_3(X) + MATHTER_MATMUL_CC_FACTOR(X, 3)
#define MATHTER_MATMUL_CC_STRIPE(CY, X) MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_CC_STRIPE_ ## CY)(X)

#define MATHTER_MATMUL_CC_ARRAY_1(CY) result.stripes[0] = MATHTER_MATMUL_CC_STRIPE(CY, 0) ;
#define MATHTER_MATMUL_CC_ARRAY_2(CY) MATHTER_MATMUL_CC_ARRAY_1(CY) result.stripes[1] = MATHTER_MATMUL_CC_STRIPE(CY, 1) ;
#define MATHTER_MATMUL_CC_ARRAY_3(CY) MATHTER_MATMUL_CC_ARRAY_2(CY) result.stripes[2] = MATHTER_MATMUL_CC_STRIPE(CY, 2) ;
#define MATHTER_MATMUL_CC_ARRAY_4(CY) MATHTER_MATMUL_CC_ARRAY_3(CY) result.stripes[3] = MATHTER_MATMUL_CC_STRIPE(CY, 3) ;

#define MATHTER_MATMUL_CC_ARRAY(CX, CY) MATHTER_MATMUL_EXPAND(MATHTER_MATMUL_CC_ARRAY_ ## CX)(CY)

#define MATHTER_MATMUL_CC_UNROLL(COLUMNS2, MATCH) if (Columns2 == COLUMNS2 && Match == MATCH ) { MATHTER_MATMUL_CC_ARRAY(COLUMNS2, MATCH) return result; }



template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::ROW_MAJOR, Packed>& lhs,
			   const Matrix<U, Match, Columns2, Order2, eMatrixLayout::ROW_MAJOR, Packed>& rhs)
	-> Matrix<V, Rows1, Columns2,  Order1, eMatrixLayout::ROW_MAJOR, Packed>
{
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed> result;

	MATHTER_MATMUL_RR_UNROLL(2, 2);
	MATHTER_MATMUL_RR_UNROLL(2, 3);
	MATHTER_MATMUL_RR_UNROLL(2, 4);

	MATHTER_MATMUL_RR_UNROLL(3, 2);
	MATHTER_MATMUL_RR_UNROLL(3, 3);
	MATHTER_MATMUL_RR_UNROLL(3, 4);

	MATHTER_MATMUL_RR_UNROLL(4, 2);
	MATHTER_MATMUL_RR_UNROLL(4, 3);
	MATHTER_MATMUL_RR_UNROLL(4, 4);

	// general algorithm
	for (int i = 0; i < Rows1; ++i) {
		result.stripes[i] = rhs.stripes[0] * lhs(i, 0);
	}
	for (int i = 0; i < Rows1; ++i) {
		for (int j = 1; j < Match; ++j) {
			result.stripes[i] += rhs.stripes[j] * lhs(i, j);
		}
	}

	return result;
}

template <class T, class U,  int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::ROW_MAJOR, Packed>& lhs,
			   const Matrix<U, Match, Columns2, Order2, eMatrixLayout::COLUMN_MAJOR, Packed>& rhs)
	-> Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed>
{
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::ROW_MAJOR, Packed> result;

	for (int j = 0; j < Columns2; ++j) {
		for (int i = 0; i < Rows1; ++i) {
			result(i, j) = Vector<T, Match, Packed>::Dot(lhs.stripes[i], rhs.stripes[j]);
		}
	}

	return result;
}

template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::COLUMN_MAJOR, Packed>& lhs,
			   const Matrix<U, Match, Columns2, Order2, eMatrixLayout::COLUMN_MAJOR, Packed>& rhs)
	->Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed>
{
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed> result;

	MATHTER_MATMUL_CC_UNROLL(2, 2);
	MATHTER_MATMUL_CC_UNROLL(2, 3);
	MATHTER_MATMUL_CC_UNROLL(2, 4);

	MATHTER_MATMUL_CC_UNROLL(3, 2);
	MATHTER_MATMUL_CC_UNROLL(3, 3);
	MATHTER_MATMUL_CC_UNROLL(3, 4);

	MATHTER_MATMUL_CC_UNROLL(4, 2);
	MATHTER_MATMUL_CC_UNROLL(4, 3);
	MATHTER_MATMUL_CC_UNROLL(4, 4);	

	// general algorithm
	for (int j = 0; j < Columns2; ++j) {
		result.stripes[j] = lhs.stripes[0] * rhs(0, j);
	}
	for (int i = 1; i < Match; ++i) {
		for (int j = 0; j < Columns2; ++j) {
			result.stripes[j] += lhs.stripes[i] * rhs(i, j);
		}
	}

	return result;
}

template <class T, class U, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, bool Packed, class V>
inline auto operator*(const Matrix<T, Rows1, Match, Order1, eMatrixLayout::COLUMN_MAJOR, Packed>& lhs,
			   const Matrix<U, Match, Columns2, Order2, eMatrixLayout::ROW_MAJOR, Packed>& rhs)
	->Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed>
{
	Matrix<V, Rows1, Columns2, Order1, eMatrixLayout::COLUMN_MAJOR, Packed> result;

	MATHTER_MATMUL_CC_UNROLL(2, 2);
	MATHTER_MATMUL_CC_UNROLL(2, 3);
	MATHTER_MATMUL_CC_UNROLL(2, 4);

	MATHTER_MATMUL_CC_UNROLL(3, 2);
	MATHTER_MATMUL_CC_UNROLL(3, 3);
	MATHTER_MATMUL_CC_UNROLL(3, 4);

	MATHTER_MATMUL_CC_UNROLL(4, 2);
	MATHTER_MATMUL_CC_UNROLL(4, 3);
	MATHTER_MATMUL_CC_UNROLL(4, 4);

	// general algorithm
	// CC algorithm is completely fine for COL_MAJOR x ROW_MAJOR
	// see that rhs is only indexed per-element, so its layout does not matter
	for (int j = 0; j < Columns2; ++j) {
		result.stripes[j] = lhs.stripes[0] * rhs(0, j);
	}
	for (int i = 1; i < Match; ++i) {
		for (int j = 0; j < Columns2; ++j) {
			result.stripes[j] += lhs.stripes[i] * rhs(i, j);
		}
	}

	return result;
}


template <class T1, class T2, int Rows1, int Match, int Columns2, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool PackedA>
inline auto operator*(const Matrix<T1, Rows1, Match, Order1, Layout1, PackedA>& lhs,
	const Matrix<T2, Match, Columns2, Order2, Layout2, PackedA>& rhs)
	->Matrix<impl::MatMulElemT<T1, T2>, Rows1, Columns2, Order1, Layout1, PackedA>
{
	return mathter::operator*<T1, T2, Rows1, Match, Columns2, Order1, Order2, PackedA, impl::MatMulElemT<T1, T2>>(lhs, rhs);
}


// Assign-multiply
template <class T1, class T2, int Dim, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed>
inline Matrix<T1, Dim, Dim, Order1, Layout1, Packed>& operator*=(Matrix<T1, Dim, Dim, Order1, Layout1, Packed>& lhs, const Matrix<T2, Dim, Dim, Order2, Layout2, Packed>& rhs) {
	lhs = operator*<T1, T2, Dim, Dim, Dim, Order1, Order2, Packed, T1>(lhs, rhs);
	return lhs;
}

#define MATHTER_MATADD_SAME_ARRAY_1(OP) result.stripes[0] = lhs.stripes[0] OP rhs.stripes[0];
#define MATHTER_MATADD_SAME_ARRAY_2(OP) MATHTER_MATADD_SAME_ARRAY_1(OP) result.stripes[1] = lhs.stripes[1] OP rhs.stripes[1];
#define MATHTER_MATADD_SAME_ARRAY_3(OP) MATHTER_MATADD_SAME_ARRAY_2(OP) result.stripes[2] = lhs.stripes[2] OP rhs.stripes[2];
#define MATHTER_MATADD_SAME_ARRAY_4(OP) MATHTER_MATADD_SAME_ARRAY_3(OP) result.stripes[3] = lhs.stripes[3] OP rhs.stripes[3];
#define MATHTER_MATADD_SAME_UNROLL(S, OP) if (result.StripeCount == S) { MATHTER_MATMUL_EXPAND(MATHTER_MATADD_SAME_ARRAY_ ## S)(OP) return result; }


// Add & sub same layout
template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout SameLayout, bool Packed, class V>
inline Matrix<U, Rows, Columns, Order1, SameLayout, Packed> operator+(
	const Matrix<T, Rows, Columns, Order1, SameLayout, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, SameLayout, Packed>& rhs)
{
	Matrix<U, Rows, Columns, Order1, SameLayout, Packed> result;

	MATHTER_MATADD_SAME_UNROLL(1, +);
	MATHTER_MATADD_SAME_UNROLL(2, +);
	MATHTER_MATADD_SAME_UNROLL(3, +);
	MATHTER_MATADD_SAME_UNROLL(4, +);

	for (int i = 0; i < result.StripeCount; ++i) {
		result.stripes[i] = lhs.stripes[i] + rhs.stripes[i];
	}
	return result;
}

template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout SameLayout, bool Packed, class V>
inline Matrix<U, Rows, Columns, Order1, SameLayout, Packed> operator-(
	const Matrix<T, Rows, Columns, Order1, SameLayout, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, SameLayout, Packed>& rhs)
{
	Matrix<U, Rows, Columns, Order1, SameLayout, Packed> result;

	MATHTER_MATADD_SAME_UNROLL(1, -);
	MATHTER_MATADD_SAME_UNROLL(2, -);
	MATHTER_MATADD_SAME_UNROLL(3, -);
	MATHTER_MATADD_SAME_UNROLL(4, -);

	for (int i = 0; i < result.StripeCount; ++i) {
		result.stripes[i] = lhs.stripes[i] - rhs.stripes[i];
	}
	return result;
}


// Add & sub opposite layout
template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed, class V, class>
inline Matrix<U, Rows, Columns, Order1, Layout1, Packed> operator+(
	const Matrix<T, Rows, Columns, Order1, Layout1, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs)
{
	Matrix<U, Rows, Columns, Order1, Layout1, Packed> result;
	for (int i = 0; i < result.RowCount(); ++i) {
		for (int j = 0; j < result.ColumnCount(); ++j) {
			result(i, j) = lhs(i, j) + rhs(i, j);
		}
	}
	return result;
}

template <class T, class U, int Rows, int Columns, eMatrixOrder Order1, eMatrixOrder Order2, eMatrixLayout Layout1, eMatrixLayout Layout2, bool Packed, class V, class>
inline Matrix<U, Rows, Columns, Order1, Layout1, Packed> operator-(
	const Matrix<T, Rows, Columns, Order1, Layout1, Packed>& lhs,
	const Matrix<U, Rows, Columns, Order2, Layout2, Packed>& rhs)
{
	Matrix<U, Rows, Columns, Order1, Layout1, Packed> result;
	for (int i = 0; i < result.RowCount(); ++i) {
		for (int j = 0; j < result.ColumnCount(); ++j) {
			result(i, j) = lhs(i, j) - rhs(i, j);
		}
	}
	return result;
}



//------------------------------------------------------------------------------
// Matrix-Scalar arithmetic
//------------------------------------------------------------------------------


template <class U, class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed, class = typename std::enable_if<impl::IsScalar<U>::value>::type>
Matrix<T, Rows, Columns, Order, Layout, Packed> operator*(U s, const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	return mat * s;
}

template <class U, class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed, class = typename std::enable_if<impl::IsScalar<U>::value>::type>
Matrix<T, Rows, Columns, Order, Layout, Packed> operator/(U s, const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	return mat / s;
}


//------------------------------------------------------------------------------
// General matrix functions
//------------------------------------------------------------------------------
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto Matrix<T, Rows, Columns, Order, Layout, Packed>::Identity() -> Matrix<T, Rows, Columns, Order, Layout, Packed> {
	Matrix<T, Rows, Columns, Order, Layout, Packed> res;

	res.SetZero();
	for (int i = 0; i < std::min(Rows, Columns); ++i) {
		res(i, i) = T(1);
	}

	return res;
}

template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
auto Matrix<T, Rows, Columns,  Order, Layout, Packed>::SetIdentity() ->Matrix<T, Rows, Columns, Order, Layout, Packed>& {
	*this = Identity();
	return *this;
}



//------------------------------------------------------------------------------
// Matrix-vector arithmetic
//------------------------------------------------------------------------------

#define MATHTER_VECMAT_ARRAY_1 result = vec(0) * mat.stripes[0];
#define MATHTER_VECMAT_ARRAY_2 MATHTER_VECMAT_ARRAY_1 result += vec(1) * mat.stripes[1];
#define MATHTER_VECMAT_ARRAY_3 MATHTER_VECMAT_ARRAY_2 result += vec(2) * mat.stripes[2];
#define MATHTER_VECMAT_ARRAY_4 MATHTER_VECMAT_ARRAY_3 result += vec(3) * mat.stripes[3];
#define MATHTER_VECMAT_UNROLL(S) if (result.Dimension() == S) { MATHTER_MATMUL_EXPAND(MATHTER_VECMAT_ARRAY_ ## S) return result; }

#define MATHTER_VECMAT_DOT_ARRAY_1 result(0) = Dot(vec, mat.stripes[0]);
#define MATHTER_VECMAT_DOT_ARRAY_2 MATHTER_VECMAT_DOT_ARRAY_1 result(1) = Dot(vec, mat.stripes[1]);
#define MATHTER_VECMAT_DOT_ARRAY_3 MATHTER_VECMAT_DOT_ARRAY_2 result(2) = Dot(vec, mat.stripes[2]);
#define MATHTER_VECMAT_DOT_ARRAY_4 MATHTER_VECMAT_DOT_ARRAY_3 result(3) = Dot(vec, mat.stripes[3]);
#define MATHTER_VECMAT_DOT_UNROLL(S) if (result.Dimension() == S) { MATHTER_MATMUL_EXPAND(MATHTER_VECMAT_DOT_ARRAY_ ## S) return result; }


// v*M
template <class Vt, class Mt, int Vd, int Mcol, eMatrixOrder Morder, bool Packed, class Rt>
inline Vector<Rt, Mcol, Packed> operator*(const Vector<Vt, Vd, Packed>& vec, const Matrix<Mt, Vd, Mcol, Morder, eMatrixLayout::ROW_MAJOR, Packed>& mat) {
	Vector<Rt, Mcol, Packed> result;
	
	MATHTER_VECMAT_UNROLL(1);
	MATHTER_VECMAT_UNROLL(2);
	MATHTER_VECMAT_UNROLL(3);
	MATHTER_VECMAT_UNROLL(4);

	result = vec(0) * mat.stripes[0];
	for (int i = 1; i < Vd; ++i) {
		result += vec(i) * mat.stripes[i];
	}
	return result;
}

template <class Vt, class Mt, int Vd, int Mcol, eMatrixOrder Morder, bool Packed, class Rt>
inline Vector<Rt, Mcol, Packed> operator*(const Vector<Vt, Vd, Packed>& vec, const Matrix<Mt, Vd, Mcol, Morder, eMatrixLayout::COLUMN_MAJOR, Packed>& mat) {
	Vector<Rt, Mcol, Packed> result;

	MATHTER_VECMAT_DOT_UNROLL(1);
	MATHTER_VECMAT_DOT_UNROLL(2);
	MATHTER_VECMAT_DOT_UNROLL(3);
	MATHTER_VECMAT_DOT_UNROLL(4);

	for (int i = 0; i < Vd; ++i) {
		result(i) = Dot(vec, mat.stripes[i]);
	}
	return result;
}

template <class Vt, class Mt, int Vd, int Mcol, eMatrixOrder Morder, eMatrixLayout Mlayout, bool Packed>
inline Vector<impl::MatMulElemT<Vt, Mt>, Mcol, Packed> operator*(const Vector<Vt, Vd, Packed>& vec, const Matrix<Mt, Vd, Mcol, Morder, Mlayout, Packed>& mat) {
	using Rt = impl::MatMulElemT<Vt, Mt>;
	return operator*<Vt, Mt, Vd, Mcol, Morder, Packed, Rt>(vec, mat);
}


// (v|1)*M
template <class Vt, class Mt, int Vd, eMatrixLayout Mlayout, eMatrixOrder Morder, bool Packed, class Rt = impl::MatMulElemT<Vt, Mt>>
Vector<Rt, Vd, Packed> operator*(const Vector<Vt, Vd, Packed>& vec, const Matrix<Mt, Vd + 1, Vd, Morder, Mlayout, Packed>& mat) {
	return (vec | Vt(1))*mat;
}

template <class Vt, class Mt, int Vd, eMatrixLayout Mlayout, eMatrixOrder Morder, bool Packed, class Rt = impl::MatMulElemT<Vt, Mt>>
Vector<Rt, Vd, Packed> operator*(const Vector<Vt, Vd, Packed>& vec, const Matrix<Mt, Vd + 1, Vd + 1, Morder, Mlayout, Packed>& mat) {
	auto res = (vec | Vt(1))*mat;
	res /= res(res.Dimension() - 1);
	return (Vector<Rt, Vd, Packed>)res;
}

// M*v
template <class Vt, class Mt, int Vd, int Mrow, eMatrixOrder Morder, bool Packed, class Rt>
inline Vector<Rt, Mrow, Packed> operator*(const Matrix<Mt, Mrow, Vd, Morder, eMatrixLayout::ROW_MAJOR, Packed>& mat, const Vector<Vt, Vd, Packed>& vec) {
	Vector<Rt, Mrow, Packed> result;

	MATHTER_VECMAT_DOT_UNROLL(1);
	MATHTER_VECMAT_DOT_UNROLL(2);
	MATHTER_VECMAT_DOT_UNROLL(3);
	MATHTER_VECMAT_DOT_UNROLL(4);

	for (int i = 0; i < Mrow; ++i) {
		result(i) = Dot(vec, mat.stripes[i]);
	}
	return result;
}

template <class Vt, class Mt, int Vd, int Mrow, eMatrixOrder Morder, bool Packed, class Rt>
inline Vector<Rt, Mrow, Packed> operator*(const Matrix<Mt, Mrow, Vd, Morder, eMatrixLayout::COLUMN_MAJOR, Packed>& mat, const Vector<Vt, Vd, Packed>& vec) {
	Vector<Rt, Mrow, Packed> result;

	MATHTER_VECMAT_UNROLL(1);
	MATHTER_VECMAT_UNROLL(2);
	MATHTER_VECMAT_UNROLL(3);
	MATHTER_VECMAT_UNROLL(4);

	result = vec(0) * mat.stripes[0];
	for (int i = 1; i < Vd; ++i) {
		result += vec(i) * mat.stripes[i];
	}
	return result;
}

template <class Vt, class Mt, int Vd, int Mrow, eMatrixOrder Morder, eMatrixLayout Mlayout, bool Packed>
Vector<impl::MatMulElemT<Vt, Mt>, Mrow, Packed> operator*(const Matrix<Mt, Mrow, Vd, Morder, Mlayout, Packed>& mat, const Vector<Vt, Vd, Packed>& vec) {
	using Rt = impl::MatMulElemT<Vt, Mt>;
	return operator*<Vt, Mt, Vd, Mrow, Morder, Packed, Rt>(mat, vec);
}



// M*(v|1)
template <class Vt, class Mt, int Vd, eMatrixLayout Mlayout, eMatrixOrder Morder, bool Packed, class Rt = impl::MatMulElemT<Vt, Mt>>
Vector<Rt, Vd, Packed> operator*(const Matrix<Mt, Vd, Vd + 1, Morder, Mlayout, Packed>& mat, const Vector<Vt, Vd, Packed>& vec) {
	return mat*(vec | Vt(1));
}

template <class Vt, class Mt, int Vd, eMatrixLayout Mlayout, eMatrixOrder Morder, bool Packed, class Rt = impl::MatMulElemT<Vt, Mt>>
Vector<Rt, Vd, Packed> operator*(const Matrix<Mt, Vd + 1, Vd + 1, Morder, Mlayout, Packed>& mat, const Vector<Vt, Vd, Packed>& vec) {
	auto res = (vec | Vt(1))*mat;
	res /= res(res.Dimension() - 1);
	return (Vector<Rt, Vd, Packed>)res;
}

// v*=M
template <class Vt, class Mt, int Vd, eMatrixOrder Morder, eMatrixLayout Layout, bool Packed>
Vector<Vt, Vd, Packed>& operator*=(Vector<Vt, Vd, Packed>& vec, const Matrix<Mt, Vd, Vd, Morder, Layout, Packed>& mat) {
	vec = operator*<Vt, Mt, Vd, Vd, Morder, Packed, Vt>(vec, mat);
	return vec;
}


// Elementwise product and division
template <class T1, class T2, int Rows, int Columns, eMatrixOrder O1, eMatrixOrder O2, eMatrixLayout L1, eMatrixLayout L2, bool Packed>
Matrix<T1, Rows, Columns, O1, L1, Packed> MulElementwise(
	const Matrix<T1, Rows, Columns, O1, L1, Packed>& lhs,
	const Matrix<T1, Rows, Columns, O1, L1, Packed>& rhs) 
{
	Matrix<T1, Rows, Columns, O1, L1, Packed> ret = lhs;
	return ret.MulElementwise(rhs);
}

template <class T1, class T2, int Rows, int Columns, eMatrixOrder O1, eMatrixOrder O2, eMatrixLayout L1, eMatrixLayout L2, bool Packed>
Matrix<T1, Rows, Columns, O1, L1, Packed> DivElementwise(
	const Matrix<T1, Rows, Columns, O1, L1, Packed>& lhs,
	const Matrix<T1, Rows, Columns, O1, L1, Packed>& rhs)
{
	Matrix<T1, Rows, Columns, O1, L1, Packed> ret = lhs;
	return ret.DivElementwise(rhs);
}


//------------------------------------------------------------------------------
// Layout compile time check
//------------------------------------------------------------------------------

template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
void Matrix<T, Rows, Columns, Order, Layout, Packed>::CheckLayoutContraints() const noexcept {
	// This list is not up to date!
	using Module1 = typename MatrixSquare<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module2 = typename MatrixRotation2D<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module3 = typename MatrixRotation3D<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module4 = typename MatrixTranslation<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module5 = typename MatrixScale<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module9 = typename MatrixShear<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module6 = typename MatrixPerspective<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module7 = typename MatrixOrthographic<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	using Module8 = typename MatrixView<T, Rows, Columns, Order, Layout, Packed>::Inherit;
	static_assert(sizeof(Matrix) == sizeof(MatrixData<T, Rows, Columns, Order, Layout, Packed>), "Your compiler did not optimize matrix class' size. Do you have empty base optimization enabled?");

	static_assert(impl::BasePtrEquals<Module1, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module2, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module3, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module4, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module5, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module6, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module7, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module8, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	static_assert(impl::BasePtrEquals<Module9, Matrix>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
}


//------------------------------------------------------------------------------
// IO
//------------------------------------------------------------------------------


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
std::ostream& operator<<(std::ostream& os, const Matrix<T, Rows, Columns, Order, Layout, Packed>& mat) {
	os << "[";
	for (int i = 0; i < mat.Height(); ++i) {
		for (int j = 0; j < mat.Width(); ++j) {
			os << mat(i, j) << (j == mat.Width() - 1 ? "" : ", ");
		}
		if (i < Rows - 1) {
			os << "; ";
		}
	}
	os << "]";
	return os;
}


template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
Matrix<T, Rows, Columns, Order, Layout, Packed> strtomat(const char* str, const char** end) {
	using MatrixT = Matrix<T, Rows, Columns, Order, Layout, Packed>;
	using VectorT = Vector<T, Columns, Packed>;
	MatrixT ret;

	const char* strproc = str;

	// parse initial bracket if any
	strproc = impl::StripSpaces(strproc);
	if (*strproc == '\0') {
		*end = str;
		return ret;
	}

	char startBracket = *strproc;
	char endBracket;
	bool hasBrackets = false;
	switch (startBracket) {
		case '(': endBracket = ')'; hasBrackets = true; ++strproc; break;
		case '[': endBracket = ']'; hasBrackets = true; ++strproc; break;
		case '{': endBracket = '}'; hasBrackets = true; ++strproc; break;
	}

	// parse rows
	for (int i = 0; i < Rows; ++i) {
		const char* rowend;
		VectorT row = strtovec<VectorT>(strproc, &rowend);
		if (rowend == strproc) {
			*end = str;
			return ret;
		}
		else {
			ret.Row(i) = row;
			strproc = rowend;
		}
		strproc = impl::StripSpaces(strproc);
		if (i < Rows - 1) {
			if (*strproc == ';') {
				++strproc;
			}
			else {
				*end = str;
				return ret;
			}
		}
	}

	// parse ending bracket corresponding to initial bracket
	if (hasBrackets) {
		strproc = impl::StripSpaces(strproc);
		if (*strproc != endBracket) {
			*end = str;
			return ret;
		}
		++strproc;
	}

	*end = strproc;
	return ret;
}

template <class MatrixT>
MatrixT strtomat(const char* str, const char** end) {
	static_assert(impl::IsMatrix<MatrixT>::value, "This type if not a matrix, dumbass.");
	
	return strtomat<
		typename impl::MatrixProperties<MatrixT>::Type,
		impl::MatrixProperties<MatrixT>::Rows,
		impl::MatrixProperties<MatrixT>::Columns,
		impl::MatrixProperties<MatrixT>::Order,
		impl::MatrixProperties<MatrixT>::Layout,
		impl::MatrixProperties<MatrixT>::Packed>
		(str, end);
}


} // namespace mathter



// Remove goddamn fucking bullshit crapware winapi macros.
#if defined(MATHTER_MINMAX)
#pragma pop_macro("min")
#pragma pop_macro("max")
#endif
