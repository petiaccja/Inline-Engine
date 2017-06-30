//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

// To remove goddamn fucking bullshit crapware winapi macros.
#if _MSC_VER && defined(min)
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#define MATHTER_MINMAX
#endif

// To enable Empty Base Class Optimization (EBCO) in MSVC.
// Why the fuck do I even have to manually enable it???
#ifdef _MSC_VER
#define MATHTER_EBCO __declspec(empty_bases)
#endif

// No, we don't support older compilers.
#if _MSC_VER < 1900
#error Visual Studio 2015 Update 2 or later version are supported.
#endif

// The rest of the code is free from obscene comments, as far as I remember.
// No guarantees.



#include <type_traits>
#include <iostream>
#include <array>
#include <cstdint>

#include "Simd.hpp"


namespace mathter {

//------------------------------------------------------------------------------
// Pre-declarations
//------------------------------------------------------------------------------

// Vector
template <class T, int Dim, bool Packed>
class Vector;

// Swizzle
template <class T, int... Indices>
class Swizzle;

// Matrix
enum class eMatrixOrder {
	PRECEDE_VECTOR,
	FOLLOW_VECTOR,
};

enum class eMatrixLayout {
	ROW_MAJOR,
	COLUMN_MAJOR,
};

template <class T, int Columns, int Rows, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
class Matrix;

// Quaternion
template <class T, bool Packed>
class Quaternion;


//------------------------------------------------------------------------------
// Template magic helper classes
//------------------------------------------------------------------------------

namespace impl {

template <template <class> class Cond, class... T>
struct All;

template <template <class> class Cond, class Head, class... Rest>
struct All<Cond, Head, Rest...> {
	static constexpr bool value = Cond<Head>::value && All<Cond, Rest...>::value;
};

template <template <class> class Cond>
struct All<Cond> {
	static constexpr bool value = true;
};


template <template <class> class Cond, class... T>
struct Any;

template <template <class> class Cond, class Head, class... Rest>
struct Any<Cond, Head, Rest...> {
	static constexpr bool value = Cond<Head>::value || Any<Cond, Rest...>::value;
};

template <template <class> class Cond>
struct Any<Cond> {
	static constexpr bool value = false;
};



template <class... T>
struct TypeList {};

template <class Tl1, class Tl2>
struct ConcatTypeList;

template <class... T, class... U>
struct ConcatTypeList<TypeList<T...>, TypeList<U...>> {
	using type = TypeList<T..., U...>;
};

template <class T, int N>
struct RepeatType {
	using type = typename std::conditional<N <= 0, TypeList<>, typename ConcatTypeList<TypeList<T>, typename RepeatType<T, N - 1>::type>::type>::type;
};


// Decide if type is Scalar, Vector or Matrix
template <class Arg>
struct IsVector {
	static constexpr bool value = false;
};
template <class T, int Dim, bool Packed>
struct IsVector<Vector<T, Dim, Packed>> {
	static constexpr bool value = true;
};
template <class Arg>
struct NotVector {
	static constexpr bool value = !IsVector<Arg>::value;
};

template <class Arg>
struct IsSwizzle {
	static constexpr bool value = false;
};
template <class T, int... Indices>
struct IsSwizzle<Swizzle<T, Indices...>> {
	static constexpr bool value = true;
};
template <class Arg>
struct NotSwizzle {
	static constexpr bool value = !IsSwizzle<Arg>::value;
};

template <class Arg>
struct IsVectorOrSwizzle {
	static constexpr bool value = IsVector<Arg>::value || IsSwizzle<Arg>::value;
};

template <class T>
struct IsMatrix {
	static constexpr bool value = false;
};
template <class T, int Rows, int Columns, eMatrixOrder Order, eMatrixLayout Layout, bool Packed>
struct IsMatrix<Matrix<T, Rows, Columns, Order, Layout, Packed>> {
	static constexpr bool value = true;
};
template <class T>
struct NotMatrix {
	static constexpr bool value = !IsMatrix<T>::value;
};

template <class Arg>
struct IsQuaternion {
	static constexpr bool value = false;
};
template <class T, bool Packed>
struct IsQuaternion<Quaternion<T, Packed>> {
	static constexpr bool value = true;
};
template <class Arg>
struct NotQuaternion {
	static constexpr bool value = !IsQuaternion<Arg>::value;
};


template <class T>
struct IsScalar {
	static constexpr bool value = !IsMatrix<T>::value && !IsVector<T>::value && !IsSwizzle<T>::value && !IsQuaternion<T>::value;
};

// Dimension of an argument (add dynamically sized vectors later)
template <class U, int Along = 0>
struct DimensionOf {
	static constexpr int value = 1;
};
template <class T, int Dim, bool Packed>
struct DimensionOf<Vector<T, Dim, Packed>, 0> {
	static constexpr int value = Dim;
};
template <class T, int... Indices>
struct DimensionOf<Swizzle<T, Indices...>> {
	static constexpr int value = sizeof...(Indices);
};

// Sum dimensions of arguments
template <class... Rest>
struct SumDimensions;

template <class Head, class... Rest>
struct SumDimensions<Head, Rest...> {
	static constexpr int value = DimensionOf<Head>::value > 0 ? DimensionOf<Head>::value + SumDimensions<Rest...>::value : -1;
};

template <>
struct SumDimensions<> {
	static constexpr int value = 0;
};


template <class T>
bool AlmostEqual(T d1, T d2) {
	if (d1 < 1e-38 && d2 < 1e-38) {
		return true;
	}
	if (d1 == 0 && d2 < 1e-4 || d2 == 0 && d1 < 1e-4) {
		return true;
	}
	T scaler = pow(T(10), floor(log10(abs(d1))));
	d1 /= scaler;
	d2 /= scaler;
	d1 *= 1000.f;
	d2 *= 1000.f;
	return round(d1) == round(d2);
}


// Check if base class's pointer equals that of derived when static_casted
template <class Base, class Derived>
class BasePtrEquals {
#if _MSC_VER > 1910
	static Derived instance;
	static constexpr void* base = static_cast<void*>(static_cast<Base*>(&instance));
public:
	static constexpr bool value = base == static_cast<void*>(&instance);
#else
public:
	static constexpr bool value = true;
#endif
};

#if _MSC_VER > 1910
template <class Base, class Derived>
Derived BasePtrEquals<Base, Derived>::instance;
#endif

} // namespace impl



//------------------------------------------------------------------------------
// Vector data containers
//------------------------------------------------------------------------------

template <class T, int... Indices>
class Swizzle {
	static constexpr int IndexTable[] = { Indices... };
	static constexpr int Dim = sizeof...(Indices);
	T* data() { return reinterpret_cast<T*>(this); }
	const T* data() const { return reinterpret_cast<const T*>(this); }
public:
	operator Vector<T, sizeof...(Indices), false>() const;
	operator Vector<T, sizeof...(Indices), true>() const;

	Swizzle& operator=(const Vector<T, sizeof...(Indices), false>& rhs);
	Swizzle& operator=(const Vector<T, sizeof...(Indices), true>& rhs);

	template <class T2, int... Indices2, typename std::enable_if<sizeof...(Indices) == sizeof...(Indices2), int>::type = 0>
	Swizzle& operator=(const Swizzle<T2, Indices2...>& rhs) {
		*this = Vector<T, sizeof...(Indices2), false>(rhs);
		return *this;
	}

	T& operator[](int idx) {
		return data()[IndexTable[idx]];
	}
	T operator[](int idx) const {
		return data()[IndexTable[idx]];
	}

	template <bool Packed = false>
	const auto ToVector() const {
		return Vector<T, Dim, Packed>(*this);
	}
protected:
	template <int... Rest, class = std::enable_if<sizeof...(Rest)==0>::type>
	void Assign(const T*) {}

	template <int Index, int... Rest>
	void Assign(const T* rhs) {
		data()[Index] = *rhs;
		return Assign<Rest...>(rhs + 1);
	}
};

// Functions they must have:
// mul, div, add, sub | vec x vec
// mul, div, add, sub | vec x scalar
// spread
// dot

// General
template <class T, int Dim, bool Packed>
class VectorData {
public:
	T data[Dim];
};


// Small vectors with x,y,z,w members
template <class T, bool Packed>
class VectorData<T, 2, Packed> {
	using ST = T;
public:
	union {
		struct { T x, y; };
		T data[2];
#include "Swizzle/Swizzle_2.inc.hpp"
	};
};

template <class T, bool Packed>
class VectorData<T, 3, Packed> {
	using ST = T;
public:
	union {
		struct { T x, y, z; };
		T data[3];
#include "Swizzle/Swizzle_3.inc.hpp"
	};
};

template <class T, bool Packed>
class VectorData<T, 4, Packed> {
	using ST = T;
public:
	union {
		struct { T x, y, z, w; };
		T data[4];
#include "Swizzle/Swizzle_4.inc.hpp"
	};
};


// Small SIMD fp32 vectors
template <>
class VectorData<float, 2, false> {
	using ST = float;
public:
	union {
		Simd<float, 2> simd;
		struct { float x, y; };
		float data[2];
#include "Swizzle/Swizzle_2.inc.hpp"
	};
};

template <>
class VectorData<float, 3, false> {
	using ST = float;
public:
	union {
		Simd<float, 4> simd;
		struct { float x, y, z; };
		float data[3];
#include "Swizzle/Swizzle_3.inc.hpp"
	};
};

template <>
class VectorData<float, 4, false> {
	using ST = float;
public:
	union {
		Simd<float, 4> simd;
		struct { float x, y, z, w; };
		float data[4];
#include "Swizzle/Swizzle_4.inc.hpp"
	};
};

template <>
class VectorData<float, 8, false> {
public:
	union {
		Simd<float, 8> simd;
		float data[8];
	};
};


// Small SIMD fp64 vectors
template <>
class VectorData<double, 2, false> {
	using ST = double;
public:
	union {
		Simd<double, 2> simd;
		struct { double x, y; };
		double data[2];
#include "Swizzle/Swizzle_2.inc.hpp"
	};
};

template <>
class VectorData<double, 3, false> {
	using ST = double;
public:
	union {
		Simd<double, 4> simd;
		struct { double x, y, z; };
		double data[3];
#include "Swizzle/Swizzle_3.inc.hpp"
	};
};

template <>
class VectorData<double, 4, false> {
	using ST = double;
public:
	union {
		Simd<double, 4> simd;
		struct { double x, y, z, w; };
		double data[4];
#include "Swizzle/Swizzle_4.inc.hpp"
	};
};



//------------------------------------------------------------------------------
// Vector basic operations
//------------------------------------------------------------------------------


template <class T>
struct has_simd {
	template <class U>
	static std::false_type test(...) { return {}; }

	template <class U>
	static decltype(U::simd) test(int) { return {}; }


	static constexpr bool value = !std::is_same<std::false_type, decltype(test<T>(0))>::value;
};


template <class T, int Dim, bool Packed, bool = has_simd<VectorData<T, Dim, Packed>>::value>
class VectorOps;


// General vector ops
template <class T, int Dim, bool Packed>
class VectorOps<T, Dim, Packed, false> {
	using VectorT = Vector<T, Dim, Packed>;

	VectorT& self() { return *reinterpret_cast<VectorT*>(this); }
	const VectorT& self() const { return *reinterpret_cast<const VectorT*>(this); }
protected:
	// Assignment
	static inline void spread(VectorT& lhs, T all) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] = all;
		}
	}

	// Vector arithmetic
	static inline void mul(VectorT& lhs, const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] *= rhs.data[i];
		}
	}
	static inline void div(VectorT& lhs, const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] /= rhs.data[i];
		};
	}
	static inline void add(VectorT& lhs, const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] += rhs.data[i];
		}
	}
	static inline void sub(VectorT& lhs, const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] -= rhs.data[i];
		}
	}
	static inline VectorT fma(const VectorT& mul1, const VectorT& mul2, const VectorT& incr) {
		VectorT ret;
		for (int i = 0; i < Dim; ++i) {
			ret.data[i] = mul1.data[i] * mul2.data[i] + incr.data[i];
		}
		return ret;
	}

	// Scalar arithmetic
	static inline void mul(VectorT& lhs, T rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] *= rhs;
		}
	}
	static inline void div(VectorT& lhs, T rhs) {
		T rcprhs = T(1) / rhs;
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] *= rcprhs;
		}
	}
	static inline void add(VectorT& lhs, T rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] += rhs;
		}
	}
	static inline void sub(VectorT& lhs, T rhs) {
		for (int i = 0; i < Dim; ++i) {
			lhs.data[i] -= rhs;
		}
	}

	// Misc
	static inline T dot(const VectorT& lhs, const VectorT& rhs) {
		T sum = 0.0f;
		for (int i = 0; i < Dim; ++i) {
			sum += lhs.data[i] * rhs.data[i];
		}
		return sum;
	}
};


// Simd accelerated vector ops
template <class T, int Dim, bool Packed>
class VectorOps<T, Dim, Packed, true> {
	using SimdT = decltype(VectorData<T, Dim, Packed>::simd);
	using VectorT = Vector<T, Dim, Packed>;

	VectorT& self() { return *reinterpret_cast<VectorT*>(this); }
	const VectorT& self() const { return *reinterpret_cast<const VectorT*>(this); }
protected:
	// Assignment
	static inline void spread(VectorT& lhs, T all) {
		lhs.simd = SimdT::spread(all);
	}

	// Vector arithmetic
	static inline void mul(VectorT& lhs, const VectorT& rhs) {
		lhs.simd = SimdT::mul(lhs.simd, rhs.simd);
	}
	static inline void div(VectorT& lhs, const VectorT& rhs) {
		lhs.simd = SimdT::div(lhs.simd, rhs.simd);
	}
	static inline void add(VectorT& lhs, const VectorT& rhs) {
		lhs.simd = SimdT::add(lhs.simd, rhs.simd);
	}
	static inline void sub(VectorT& lhs, const VectorT& rhs) {
		lhs.simd = SimdT::sub(lhs.simd, rhs.simd);
	}
	static inline VectorT fma(const VectorT& mul1, const VectorT& mul2, const VectorT& incr) {
		VectorT ret;
		ret.simd = SimdT::mad(mul1.simd, mul2.simd, incr.simd);
		return ret;
	}

	// Scalar arithmetic
	static inline void mul(VectorT& lhs, T rhs) {
		lhs.simd = SimdT::mul(lhs.simd, rhs);
	}
	static inline void div(VectorT& lhs, T rhs) {
		lhs.simd = SimdT::div(lhs.simd, rhs);
	}
	static inline void add(VectorT& lhs, T rhs) {
		lhs.simd = SimdT::add(lhs.simd, rhs);
	}
	static inline void sub(VectorT& lhs, T rhs) {
		lhs.simd = SimdT::sub(lhs.simd, rhs);
	}

	// Misc
	static inline T dot(const VectorT& lhs, const VectorT& rhs) {
		return SimdT::dot<Dim>(lhs.simd, rhs.simd);
	}
};


//------------------------------------------------------------------------------
// Vector special operations
//------------------------------------------------------------------------------

// Note: VectorSpecialOps inherits from VectorOps instead of making
// Vector directly inherit from both because MSVC sucks hard with EBCO, and bloats
// Vector with useless bullshit twice the size it should be.

template <class T, int Dim, bool Packed>
class VectorSpecialOps : public VectorOps<T, Dim, Packed> {
	using VectorT = Vector<T, Dim, Packed>;
public:
	template <class... Args>
	static VectorT Cross(const VectorT& head, Args&&... args);

	static VectorT Cross(const std::array<const VectorT*, Dim - 1>& args);
};

template <class T, bool Packed>
class VectorSpecialOps<T, 2, Packed> : public VectorOps<T, 2, Packed> {
	using VectorT = Vector<T, 2, Packed>;
public:
	static VectorT Cross(const VectorT& arg) {
		return VectorT(-arg.y,
					   arg.x);
	}

	static VectorT Cross(const std::array<const VectorT*, 1>& arg) {
		return Cross(*(arg[0]));
	}
};

template <class T, bool Packed>
class VectorSpecialOps<T, 3, Packed> : public VectorOps<T, 3, Packed> {
	using VectorT = Vector<T, 3, Packed>;
public:
	static VectorT Cross(const VectorT& lhs, const VectorT& rhs) {
		return VectorT(lhs.y * rhs.z - lhs.z * rhs.y,
					   lhs.z * rhs.x - lhs.x * rhs.z,
					   lhs.x * rhs.y - lhs.y * rhs.x);
	}

	static VectorT Cross(const std::array<const VectorT*, 2>& args) {
		return Cross(*(args[0]), *(args[1]));
	}
};



//------------------------------------------------------------------------------
// General Vector class - no specializations needed
//------------------------------------------------------------------------------

template <class T, int Dim, bool Packed = false>
class MATHTER_EBCO Vector
	: public VectorSpecialOps<T, Dim, Packed>,
	public VectorData<T, Dim, Packed>
{
	static_assert(Dim >= 1, "Dimension must be positive integer.");

	// Make a call to this function in EVERY constructor of the Vector class.
	// These checks must be put in a separate function instead of class scope because the full definition
	// of the Vector class is required to determine memory layout.
	void CheckLayoutContraints() {
		static_assert(sizeof(Vector<T, Dim, Packed>) == sizeof(VectorData<T, Dim, Packed>), "Your compiler did not optimize vector class' size. Do you have empty base optimization enabled?");
		static_assert(impl::BasePtrEquals<VectorData<T, Dim, Packed>, Vector>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
		static_assert(impl::BasePtrEquals<VectorOps<T, Dim, Packed>, Vector>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
		static_assert(impl::BasePtrEquals<VectorSpecialOps<T, Dim, Packed>, Vector>::value, "Your compiler did not lay out derived class' memory correctly. Empty base class' offset must be zero in derived.");
	}
public:
	static void DumpLayout(std::ostream& os) {
		Vector* ptr = reinterpret_cast<Vector*>(1000);
		os << "VectorData:       " << (intptr_t)static_cast<VectorData<float, 4, false>*>(ptr) - 1000 << " -> " << sizeof(VectorData<float, 4, false>) << endl;
		os << "VectorOps:        " << (intptr_t)static_cast<VectorOps<float, 4, false>*>(ptr) - 1000 << " -> " << sizeof(VectorOps<float, 4, false>) << endl;
		os << "VectorSpecialOps: " << (intptr_t)static_cast<VectorSpecialOps<float, 4, false>*>(ptr) - 1000 << " -> " << sizeof(VectorSpecialOps<float, 4, false>) << endl;
		os << "Vector:           " << (intptr_t)static_cast<Vector<float, 4, false>*>(ptr) - 1000 << " -> " << sizeof(Vector<float, 4, false>) << endl;
	}

	//--------------------------------------------
	// Data constructors
	//--------------------------------------------

	// Default ctor
	Vector() { 
		CheckLayoutContraints(); 
	}

	Vector(const Vector& rhs) :	VectorData<T, Dim, Packed>(rhs)	{ 
		CheckLayoutContraints();
	}

	Vector& operator=(const Vector& rhs) {
		VectorData<T, Dim, Packed>::operator=(rhs);
		return *this;
	}


	// All element same ctor
	explicit Vector(T all) {
		CheckLayoutContraints();
		VectorOps<T, Dim, Packed>::spread(*this, all);
	}

	// T array ctor
	template <class U>
	explicit Vector(const U* data) {
		CheckLayoutContraints();
		for (int i = 0; i < Dim; ++i) {
			this->data[i] = data[i];
		}
	}

	//--------------------------------------------
	// Homogeneous up- and downcast
	//--------------------------------------------

	explicit operator Vector<T, Dim - 1, Packed>() {
		return Vector<T, Dim - 1, Packed>(this->data);
	}

	explicit operator Vector<T, Dim + 1, Packed>() {
		return Vector<T, Dim + 1, Packed>(*this, 1);
	}

	//--------------------------------------------
	// Copy, assignment, set
	//--------------------------------------------

	// NOTE: somehow msvc 2015 is buggy and cannot compile sizeof... checks for ctors as in Set

	// Scalar concat constructor
	template <class H1, class H2, class... Scalars, typename std::enable_if<impl::All<impl::IsScalar, H1, H2, Scalars...>::value && impl::SumDimensions<H1, H2, Scalars...>::value == Dim, int>::type = 0>
	Vector(H1 h1, H2 h2, Scalars... scalars) {
		CheckLayoutContraints();
		Assign(0, h1, h2, scalars...);
	}

	// Generalized concat constructor
	template <class H1, class... Mixed, typename std::enable_if<impl::Any<impl::IsVectorOrSwizzle, H1, Mixed...>::value && impl::SumDimensions<H1, Mixed...>::value == Dim, int>::type = 0>
	Vector(const H1& h1, const Mixed&... mixed) {
		CheckLayoutContraints();
		Assign(0, h1, mixed...);
	}

	// Scalar concat set
	template <class... Scalars, typename std::enable_if<((sizeof...(Scalars) > 1) && impl::All<impl::IsScalar, Scalars...>::value), int>::type = 0>
	Vector& Set(Scalars... scalars) {
		static_assert(impl::SumDimensions<Scalars...>::value == Dim, "Arguments must match vector dimension.");
		Assign(0, scalars...);
		return *this;
	}

	// Generalized concat set
	template <class... Mixed, typename std::enable_if<(sizeof...(Mixed) > 0) && impl::Any<impl::IsVectorOrSwizzle, Mixed...>::value, int>::type = 0>
	Vector& Set(const Mixed&... mixed) {
		static_assert(impl::SumDimensions<Mixed...>::value == Dim, "Arguments must match vector dimension.");
		Assign(0, mixed...);
		return *this;
	}

	// Set all members to certain type
	Vector& Spread(T all) {
		VectorOps<T, Dim, Packed>::spread(*this, all);

		return *this;
	}

	//--------------------------------------------
	// Properties
	//--------------------------------------------

	constexpr int Dimension() const {
		return Dim;
	}


	//--------------------------------------------
	// Accessors
	//--------------------------------------------

	T operator[](int idx) const {
		return data[idx];
	}

	T& operator[](int idx) {
		return data[idx];
	}

	T operator()(int idx) const {
		return data[idx];
	}

	T& operator()(int idx) {
		return data[idx];
	}

	const T* cbegin() const {
		return data;
	}
	const T* begin() const {
		return data;
	}
	T* begin() {
		return data;
	}
	const T* cend() const {
		return data + Dim;
	}
	const T* end() const {
		return data + Dim;
	}
	T* end() {
		return data + Dim;
	}


	const T* Data() const {
		return data;
	}
	T* Data() {
		return data;
	}


	//--------------------------------------------
	// Compare
	//--------------------------------------------

	bool operator==(const Vector& rhs) const {
		bool same = data[0] == rhs.data[0];
		for (int i = 1; i < Dim; ++i) {
			same = same && data[i] == rhs.data[i];
		}
		return same;
	}

	bool operator!=(const Vector& rhs) const {
		return !operator==(rhs);
	}

	template <class = typename std::enable_if<std::is_floating_point<T>::value>::type>
	bool AlmostEqual(const Vector& rhs) const {
		bool same = true;
		for (int i = 0; i < Dim; ++i) {
			T d1 = data[i], d2 = rhs.data[i];
			bool memberEqual = impl::AlmostEqual(d1, d2);
			same = same && memberEqual;
		}
		return same;
	}

	//--------------------------------------------
	// Arithmetic
	//--------------------------------------------


	// Vector assign arithmetic
	inline Vector& operator*=(const Vector& rhs) {
		mul(*this, rhs);
		return *this;
	}

	inline Vector& operator/=(const Vector& rhs) {
		div(*this, rhs);
		return *this;
	}

	inline Vector& operator+=(const Vector& rhs) {
		add(*this, rhs);
		return *this;
	}

	inline Vector& operator-=(const Vector& rhs) {
		sub(*this, rhs);
		return *this;
	}

	inline Vector operator-() const {
		return (*this) * T(-1);
	}

	inline Vector operator+() const {
		return *this;
	}

	// Vector arithmetic
	inline Vector operator*(const Vector& rhs) const { return Vector(*this) *= rhs; }
	inline Vector operator/(const Vector& rhs) const { return Vector(*this) /= rhs; }
	inline Vector operator+(const Vector& rhs) const { return Vector(*this) += rhs; }
	inline Vector operator-(const Vector& rhs) const { return Vector(*this) -= rhs; }

	// Scalar assign arithmetic
	inline Vector& operator*=(T rhs) {
		mul(*this, rhs);
		return *this;
	}

	inline Vector& operator/=(T rhs) {
		div(*this, rhs);
		return *this;
	}

	inline Vector& operator+=(T rhs) {
		add(*this, rhs);
		return *this;
	}

	inline Vector& operator-=(T rhs) {
		sub(*this, rhs);
		return *this;
	}

	// Scalar arithmetic
	inline Vector operator*(T rhs) const { return Vector(*this) *= rhs; }
	inline Vector operator/(T rhs) const { return Vector(*this) /= rhs; }
	inline Vector operator+(T rhs) const { return Vector(*this) += rhs; }
	inline Vector operator-(T rhs) const { return Vector(*this) -= rhs; }

	static inline Vector MultiplyAdd(const Vector& a, const Vector& b, const Vector& c) {
		return fma(a, b, c);
	}


	//--------------------------------------------
	// Common functions
	//--------------------------------------------

	void Normalize() {
		T l = Length();
		operator/=(l);
	}

	Vector Normalized() const {
		Vector v = *this;
		v.Normalize();
		return v;
	}

	bool IsNormalized() const {
		T n = LengthSquared();
		return T(0.9999) <= n && n <= T(1.0001);
	}


	static T Dot(const Vector& lhs, const Vector& rhs) {
		return dot(lhs, rhs);
	}

	template <class T2, bool Packed2, typename std::enable_if<!std::is_same<T, T2>::value, int>::type = 0>
	static auto Dot(const Vector& lhs, const Vector<T2, Dim, Packed2>& rhs) {
		auto s = lhs.data[0] * rhs.data[0];
		for (int i = 1; i < Dim; ++i) {
			s = lhs.data[i] * rhs.data[i] + s;
		}
	}


	T LengthSquared() const {
		return Dot(*this, *this);
	}

	T Length() const {
		return sqrt(LengthSquared());
	}

protected:
	//--------------------------------------------
	// Helpers
	//--------------------------------------------

	// Get nth element of an argument
	template <class U>
	struct GetVectorElement {
		static U Get(const U& u, int idx) { return u; }
	};
	template <class U, int E, bool Packed>
	struct GetVectorElement<Vector<U, E, Packed>> {
		static U Get(const Vector<U, E, Packed>& u, int idx) { return u.data[idx]; }
	};
	template <class U, int... Indices>
	struct GetVectorElement<Swizzle<U, Indices...>> {
		static U Get(const Swizzle<T, Indices...>& u, int idx) { return u[idx]; }
	};

	// Assign
	// Scalar concat assign
	template <class Head, class... Scalars, typename std::enable_if<impl::All<impl::IsScalar, Head, Scalars...>::value, int>::type = 0>
	void Assign(int idx, Head head, Scalars... scalars) {
		data[idx] = (T)head;
		Assign(idx + 1, scalars...);
	}

	// Generalized concat assign
	template <class Head, class... Mixed, typename std::enable_if<impl::Any<impl::IsVectorOrSwizzle, Head, Mixed...>::value, int>::type = 0>
	void Assign(int idx, const Head& head, const Mixed&... mixed) {
		for (int i = 0; i < impl::DimensionOf<Head>::value; ++i) {
			data[idx] = GetVectorElement<Head>::Get(head, i);
			++idx;
		}
		Assign(idx, mixed...);
	}

	// Assign terminator, fill stuff with zeros
	void Assign(int idx) {
		for (; idx < Dim; idx++) {
			data[idx] = T(0);
		}
	}
};


//------------------------------------------------------------------------------
// External Vector function
//------------------------------------------------------------------------------


// Vector-Scalar arithmetic
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator*(U lhs, const Vector<T, Dim, Packed>& rhs) {
	return rhs*(T)lhs;
}

template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator+(U lhs, const Vector<T, Dim, Packed>& rhs) {
	return rhs + (T)lhs;
}

template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator-(U lhs, const Vector<T, Dim, Packed>& rhs) {
	return rhs - (T)lhs;
}

// Divide scalar by vector
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator/(U lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> res(lhs);
	res /= rhs;
	return res;
}



//------------------------------------------------------------------------------
// Common vector operations as globals
//------------------------------------------------------------------------------


template <class T, class U, int Dim, bool Packed1, bool Packed2>
auto Dot(const Vector<T, Dim, Packed1>& lhs, const Vector<U, Dim, Packed2>& rhs) {
	return Vector<T, Dim, Packed1>::Dot(lhs, rhs);
}


template <class T, int Dim, bool Packed, class... Vectors>
auto Cross(const Vector<T, Dim, Packed>& head, const Vectors&... vectors) {
	return Vector<T, Dim, Packed>::Cross(head, vectors...);
}

template <class T, int Dim, bool Packed>
auto Cross(const std::array<const Vector<T, Dim, Packed>*, Dim - 1>& vectors) {
	return Vector<T, Dim, Packed>::Cross(vectors);
}


template <class T, class U, int Dim, bool Packed1, bool Packed2>
auto Distance(const Vector<T, Dim, Packed1>& lhs, const Vector<U, Dim, Packed2>& rhs) {
	return (lhs - rhs).Length();
}

template <class T, int Dim, int Packed>
auto Normalized(const Vector<T, Dim, Packed>& arg) {
	return arg.Normalized();
}



//------------------------------------------------------------------------------
// Vector concatenation
//------------------------------------------------------------------------------
template <class T, int Dim, bool Packed, class U>
mathter::Vector<T, Dim + 1, Packed> operator|(const mathter::Vector<T, Dim, Packed>& lhs, U rhs) {
	mathter::Vector<T, Dim + 1, Packed> ret;
	for (int i = 0; i < Dim; ++i) {
		ret(i) = lhs(i);
	}
	ret(Dim) = rhs;
	return ret;
}

template <class T1, int Dim1, class T2, int Dim2, bool Packed>
mathter::Vector<T1, Dim1 + Dim2, Packed> operator|(const mathter::Vector<T1, Dim1, Packed>& lhs, const mathter::Vector<T2, Dim2, Packed>& rhs) {
	mathter::Vector<T1, Dim1 + Dim2, Packed> ret;
	for (int i = 0; i < Dim1; ++i) {
		ret(i) = lhs(i);
	}
	for (int i = 0; i < Dim2; ++i) {
		ret(Dim1 + i) = rhs(i);
	}
	return ret;
}

template <class T, int Dim, bool Packed, class U>
mathter::Vector<T, Dim + 1, Packed> operator|(U lhs, const mathter::Vector<T, Dim, Packed>& rhs) {
	mathter::Vector<T, Dim + 1, Packed> ret;
	ret(0) = lhs;
	for (int i = 0; i < Dim; ++i) {
		ret(i + 1) = rhs(i);
	}
	return ret;
}

template <class T1, int... Indices1, class T2, int... Indices2>
Vector<T1, sizeof...(Indices2)+sizeof...(Indices2), false> operator|(const Swizzle<T1, Indices1...>& lhs, const Swizzle<T2, Indices2...>& rhs) {
	return Vector<T1, sizeof...(Indices1), false>(lhs) | Vector<T1, sizeof...(Indices2), false>(rhs);
}
template <class T1, int... Indices1, class T2, int Dim, bool Packed>
Vector<T1, sizeof...(Indices1)+Dim, Packed> operator|(const Swizzle<T1, Indices1...>& lhs, const Vector<T2, Dim, Packed>& rhs) {
	return Vector<T1, sizeof...(Indices1), Packed>(lhs) | rhs;
}
template <class T1, int... Indices1, class T2, int Dim, bool Packed>
Vector<T1, sizeof...(Indices1)+Dim, Packed> operator|(const Vector<T2, Dim, Packed>& lhs, const Swizzle<T1, Indices1...>& rhs) {
	return lhs | Vector<T1, sizeof...(Indices1), false>(rhs);
}
template <class T1, int... Indices1, class U>
Vector<T1, sizeof...(Indices1)+1, false> operator|(const Swizzle<T1, Indices1...>& lhs, U rhs) {
	return Vector<T1, sizeof...(Indices1), false>(lhs) | rhs;
}
template <class T1, int... Indices1, class U>
Vector<T1, sizeof...(Indices1)+1, false> operator|(U lhs, const Swizzle<T1, Indices1...>& rhs) {
	return lhs | Vector<T1, sizeof...(Indices1), false>(rhs);
}



} // namespace mathter



// Generalized cross-product unfortunately needs matrix determinant.
#include "Matrix.hpp"

namespace mathter {


template <class T, int Dim, bool Packed>
auto VectorSpecialOps<T, Dim, Packed>::Cross(const std::array<const VectorT*, Dim - 1>& args) -> VectorT {
	VectorT result;
	Matrix<T, Dim - 1, Dim - 1> detCalc;

	// Calculate elements of result on-by-one
	int sign = 2 * (Dim % 2) - 1;
	for (int base = 0; base < result.Dimension(); ++base, sign *= -1) {
		// Fill up sub-matrix the determinant of which yields the coefficient of base-vector.
		for (int j = 0; j < base; ++j) {
			for (int i = 0; i < detCalc.RowCount(); ++i) {
				detCalc(i, j) = (*(args[i]))[j];
			}
		}
		for (int j = base + 1; j < result.Dimension(); ++j) {
			for (int i = 0; i < detCalc.RowCount(); ++i) {
				detCalc(i, j - 1) = (*(args[i]))[j];
			}
		}

		T coefficient = T(sign) * detCalc.Determinant();
		result(base) = coefficient;
	}

	return result;
}


template <class T, int Dim, bool Packed>
template <class... Args>
auto VectorSpecialOps<T, Dim, Packed>::Cross(const VectorT& head, Args&&... args) -> VectorT {
	static_assert(1 + sizeof...(args) == Dim - 1, "Number of arguments must be (Dimension - 1).");

	std::array<const VectorT*, Dim - 1> vectors = { &head, &args... };
	return Cross(vectors);
}



//------------------------------------------------------------------------------
// Swizzle
//------------------------------------------------------------------------------
template <class T, int... Indices>
Swizzle<T, Indices...>::operator Vector<T, sizeof...(Indices), false>() const {
	return Vector<T, sizeof...(Indices), false>(data()[Indices]...);
}
template <class T, int... Indices>
Swizzle<T, Indices...>::operator Vector<T, sizeof...(Indices), true>() const {
	return Vector<T, sizeof...(Indices), true>(data()[Indices]...);
}

template <class T, int... Indices>
Swizzle<T, Indices...>& Swizzle<T, Indices...>::operator=(const Vector<T, sizeof...(Indices), false>& rhs) {
	if (data() != rhs.data) {
		Assign<Indices...>(rhs.data);
	}
	else {
		Vector<T, sizeof...(Indices), false> tmp = rhs;
		*this = tmp;
	}
	return *this;
}
template <class T, int... Indices>
Swizzle<T, Indices...>& Swizzle<T, Indices...>::operator=(const Vector<T, sizeof...(Indices), true>& rhs) {
	if (data() != rhs.data) {
		Assign<Indices...>(rhs.data);
	}
	else {
		Vector<T, sizeof...(Indices), false> tmp = rhs;
		*this = tmp;
	}
	return *this;
}



} // namespace mathter




//------------------------------------------------------------------------------
// IO
//------------------------------------------------------------------------------

template <class T, int Dim, bool Packed>
std::ostream& operator<<(std::ostream& os, const mathter::Vector<T, Dim, Packed>& v) {
	os << "[";
	for (int x = 0; x < Dim; ++x) {
		os << v(x) << (x == Dim - 1 ? "" : "\t");
	}
	os << "]";
	return os;
}


// Remove goddamn fucking bullshit crapware winapi macros.
#if defined(MATHTER_MINMAX)
#pragma pop_macro("min")
#pragma pop_macro("max")
#endif
