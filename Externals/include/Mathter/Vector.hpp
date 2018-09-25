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
// Why the fuck do I even have to manually enable it??? Get you shit together Microsoft!
#ifdef _MSC_VER
#define MATHTER_EBCO __declspec(empty_bases)
#else
#define MATHTER_EBCO
#endif

// No, we don't support older compilers.
#if _MSC_VER && _MSC_VER < 1900
#error Visual Studio 2015 Update 2 or later versions are supported.
#endif

// The rest of the code is free from obscene comments, as far as I remember.
// No guarantees.



#include <type_traits>
#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>
#include <cassert>

#include "DefinitionsUtil.hpp"

#include "Simd.hpp"
#include "Utility.hpp"



namespace mathter {


//------------------------------------------------------------------------------
// Vector data containers
//------------------------------------------------------------------------------


/// <summary> Enables element swizzling (reordering elements) for vectors. </summary>
/// <remarks>
/// To access swizzlers, use the xx, xy, xyz and similar elements of vectors.
/// Swizzlers can be used with assignments, concatenation, casting and constructors.
/// To perform arithmetic, cast swizzlers to corresponding vector type.
/// </remarks>
template <class T, int... Indices>
class Swizzle {
	static constexpr int IndexTable[] = { Indices... };
	static constexpr int Dim = sizeof...(Indices);
	T* data() { return reinterpret_cast<T*>(this); }
	const T* data() const { return reinterpret_cast<const T*>(this); }
public:
	/// <summary> Builds the swizzled vector object. </summary>
	operator Vector<T, sizeof...(Indices), false>() const;
	/// <summary> Builds the swizzled vector object. </summary>
	operator Vector<T, sizeof...(Indices), true>() const;

	/// <summary> Sets the parent vector's elements from the right-side argument. </summary>
	/// <remarks> 
	/// Example: b = {1,2,3}; a.yxz = b; -> a contains {2,1,3}.
	/// You don't have to worry about aliasing (a.xyz = a is totally fine).
	/// </remarks>
	Swizzle& operator=(const Vector<T, sizeof...(Indices), false>& rhs);
	/// <summary> Sets the parent vector's elements from the right-side argument. </summary>
	/// <remarks> 
	/// Example: b = {1,2,3}; a.yxz = b; -> a contains {2,1,3}.
	/// You don't have to worry about aliasing (a.xyz = a is totally fine).
	/// </remarks>
	Swizzle& operator=(const Vector<T, sizeof...(Indices), true>& rhs);

	/// <summary> Sets the parent vector's elements from the right-side argument. </summary>
	/// <remarks> 
	/// Example: b = {1,2,3}; a.yxz = b.xyz; -> a contains {2,1,3}.
	/// You don't have to worry about aliasing (a.xyz = a.zyx is totally fine).
	/// </remarks>
	template <class T2, int... Indices2, typename std::enable_if<sizeof...(Indices) == sizeof...(Indices2), int>::type = 0>
	Swizzle& operator=(const Swizzle<T2, Indices2...>& rhs) {
		*this = Vector<T, sizeof...(Indices2), false>(rhs);
		return *this;
	}

	/// <summary> Returns the nth element of the swizzled vector. Example: v.zxy[2] returns y. </summary>
	T& operator[](int idx) {
		assert(idx < Dim);
		return data()[IndexTable[idx]];
	}
	/// <summary> Returns the nth element of the swizzled vector. Example: v.zxy[2] returns y. </summary>
	T operator[](int idx) const {
		assert(idx < Dim);
		return data()[IndexTable[idx]];
	}

	/// <summary> Returns the nth element of the swizzled vector. Example: v.zxy(2) returns y. </summary>
	T& operator()(int idx) {
		assert(idx < Dim);
		return data()[IndexTable[idx]];
	}
	/// <summary> Returns the nth element of the swizzled vector. Example: v.zxy(2) returns y. </summary>
	T operator()(int idx) const {
		assert(idx < Dim);
		return data()[IndexTable[idx]];
	}

	/// <summary> Builds the swizzled vector object. </summary>
	template <bool Packed = false>
	const auto ToVector() const {
		return Vector<T, Dim, Packed>(*this);
	}
protected:
	template <int... Rest, class = typename std::enable_if<sizeof...(Rest)==0>::type>
	void Assign(const T*) {}

	template <int Index, int... Rest>
	void Assign(const T* rhs) {
		data()[Index] = *rhs;
		return Assign<Rest...>(rhs + 1);
	}
};

template <class T, int... Indices>
constexpr int Swizzle<T, Indices...>::IndexTable[];


// Mental note to myself:
// The C++ standard requires std::complex numbers to be like
// class complex { T real; T imag; }
// so they can be cast to an array of T and indexed even for real and odd for imag parts.


// General
template <class T, int Dim, bool Packed>
class VectorData {
public:
	T data[Dim]; /// <summary> Raw array containing the elements. </summary>
};


// Small vectors with x,y,z,w members
template <class T, bool Packed>
class VectorData<T, 2, Packed> {
	using ST = T;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) {
		for (int i = 0; i<2; ++i) { data[i] = rhs.data[i]; }
	}
	VectorData& operator=(const VectorData& rhs) {
		for (int i = 0; i<2; ++i) { data[i] = rhs.data[i]; }
		return *this;
	}
	union {
		struct { T x, y; };
		T data[2]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_2.inc.hpp"
	};
};

template <class T, bool Packed>
class VectorData<T, 3, Packed> {
	using ST = T;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) {
		for (int i = 0; i<3; ++i) { data[i] = rhs.data[i]; }
	}
	VectorData& operator=(const VectorData& rhs) {
		for (int i = 0; i<3; ++i) { data[i] = rhs.data[i]; }
		return *this;
	}
	union {
		struct { T x, y, z; };
		T data[3]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_3.inc.hpp"
	};
};

template <class T, bool Packed>
class VectorData<T, 4, Packed> {
	using ST = T;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) {
		for (int i = 0; i<4; ++i) { data[i] = rhs.data[i]; }
	}
	VectorData& operator=(const VectorData& rhs) {
		for (int i = 0; i<4; ++i) { data[i] = rhs.data[i]; }
		return *this;
	}
	union {
		struct { T x, y, z, w; };
		T data[4]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_4.inc.hpp"
	};
};


// Small SIMD fp32 vectors
template <>
class VectorData<float, 2, false> {
	using ST = float;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<float, 2> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		struct { float x, y; };
		float data[2]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_2.inc.hpp"
	};
};

template <>
class VectorData<float, 3, false> {
	using ST = float;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<float, 4> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		struct { float x, y, z; };
		float data[3]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_3.inc.hpp"
	};
};

template <>
class VectorData<float, 4, false> {
	using ST = float;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<float, 4> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		struct { float x, y, z, w; };
		float data[4]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_4.inc.hpp"
	};
};

template <>
class VectorData<float, 8, false> {
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<float, 8> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		float data[8]; /// <summary> Raw array containing the elements. </summary>
	};
};


// Small SIMD fp64 vectors
template <>
class VectorData<double, 2, false> {
	using ST = double;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<double, 2> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		struct { double x, y; };
		double data[2]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_2.inc.hpp"
	};
};

template <>
class VectorData<double, 3, false> {
	using ST = double;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<double, 4> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		struct { double x, y, z; };
		double data[3]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_3.inc.hpp"
	};
};

template <>
class VectorData<double, 4, false> {
	using ST = double;
public:
	VectorData() {}
	VectorData(const VectorData& rhs) { simd = rhs.simd; }
	VectorData& operator=(const VectorData& rhs) { simd = rhs.simd; return *this; }
	union {
		Simd<double, 4> simd; /// <summary> Leave this member alone. You can't fuck it up though. </summary>
		struct { double x, y, z, w; };
		double data[4]; /// <summary> Raw array containing the elements. </summary>
#include "Swizzle/Swizzle_4.inc.hpp"
	};
};



//------------------------------------------------------------------------------
// Vector basic operations
//------------------------------------------------------------------------------

namespace impl {
	template <class T>
	struct HasSimd {
		template <class U>
		static std::false_type test(...) { return {}; }

		template <class U>
		static decltype(U::simd) test(int) { return {}; }


		static constexpr bool value = !std::is_same<std::false_type, decltype(test<T>(0))>::value;
	};
}


template <class T, int Dim, bool Packed, bool = impl::HasSimd<VectorData<T, Dim, Packed>>::value>
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

	// Misc
	static inline T dot(const VectorT& lhs, const VectorT& rhs) {
		T sum = T(0);
		for (int i = 0; i < Dim; ++i) {
			sum += lhs.data[i] * rhs.data[i];
		}
		return sum;
	}

	// Arithmetic operators
	/// <summary> Elementwise (Hadamard) vector product. </summary>
	inline VectorT operator*(const VectorT& rhs) const {
		VectorT result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = self().data[i] * rhs.data[i];
		}
		return result;
	}
	/// <summary> Elementwise vector division. </summary>
	inline VectorT operator/(const VectorT& rhs) const {
		VectorT result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = self().data[i] / rhs.data[i];
		}
		return result;
	}
	/// <summary> Elementwise vector addition. </summary>
	inline VectorT operator+(const VectorT& rhs) const {
		VectorT result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = self().data[i] + rhs.data[i];
		}
		return result;
	}
	/// <summary> Elementwise vector subtraction. </summary>
	inline VectorT operator-(const VectorT& rhs) const {
		VectorT result;
		for (int i = 0; i < Dim; ++i) {
			result[i] = self().data[i] - rhs.data[i];
		}
		return result;
	}

	// Vector assign arithmetic
	/// <summary> Elementwise (Hadamard) vector product. </summary>
	inline VectorT& operator*=(const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] *= rhs.data[i];
		}
		return self();
	}

	/// <summary> Elementwise vector division. </summary>
	inline VectorT& operator/=(const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] /= rhs.data[i];
		}
		return self();
	}

	/// <summary> Elementwise vector addition. </summary>
	inline VectorT& operator+=(const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] += rhs.data[i];
		}
		return self();
	}

	/// <summary> Elementwise vector subtraction. </summary>
	inline VectorT& operator-=(const VectorT& rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] -= rhs.data[i];
		}
		return self();
	}

	// Scalar assign arithmetic
	/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
	inline VectorT& operator*=(T rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] *= rhs;
		}
		return self();
	}

	/// <summary> Scales the vector by 1/<paramref name="rhs"/>. </summary>
	inline VectorT& operator/=(T rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] /= rhs;
		}
		return self();
	}

	/// <summary> Adds <paramref name="rhs"/> to each element of the vector. </summary>
	inline VectorT& operator+=(T rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] += rhs;
		}
		return self();
	}

	/// <summary> Subtracts <paramref name="rhs"/> from each element of the vector. </summary>
	inline VectorT& operator-=(T rhs) {
		for (int i = 0; i < Dim; ++i) {
			self().data[i] -= rhs;
		}
		return self();
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

	// Misc
	static inline T dot(const VectorT& lhs, const VectorT& rhs) {
		return SimdT::template dot<Dim>(lhs.simd, rhs.simd);
	}

	// Arithmetic operators
	/// <summary> Elementwise (Hadamard) vector product. </summary>
	inline VectorT operator*(const VectorT& rhs) const {
		VectorT result;
		result.simd = SimdT::mul(self().simd, rhs.simd);
		return result;
	}
	/// <summary> Elementwise vector division. </summary>
	inline VectorT operator/(const VectorT& rhs) const {
		VectorT result;
		result.simd = SimdT::div(self().simd, rhs.simd);
		return result;
	}
	/// <summary> Elementwise vector addition. </summary>
	inline VectorT operator+(const VectorT& rhs) const {
		VectorT result;
		result.simd = SimdT::add(self().simd, rhs.simd);
		return result;
	}
	/// <summary> Elementwise vector subtraction. </summary>
	inline VectorT operator-(const VectorT& rhs) const {
		VectorT result;
		result.simd = SimdT::sub(self().simd, rhs.simd);
		return result;
	}

	// Vector assign arithmetic
	/// <summary> Elementwise (Hadamard) vector product. </summary>
	inline VectorT& operator*=(const VectorT& rhs) {
		self().simd = SimdT::mul(self().simd, rhs.simd);
		return self();
	}

	/// <summary> Elementwise vector division. </summary>
	inline VectorT& operator/=(const VectorT& rhs) {
		self().simd = SimdT::div(self().simd, rhs.simd);
		return self();
	}

	/// <summary> Elementwise vector addition. </summary>
	inline VectorT& operator+=(const VectorT& rhs) {
		self().simd = SimdT::add(self().simd, rhs.simd);
		return self();
	}

	/// <summary> Elementwise vector subtraction. </summary>
	inline VectorT& operator-=(const VectorT& rhs) {
		self().simd = SimdT::sub(self().simd, rhs.simd);
		return self();
	}

	// Scalar assign arithmetic
	/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
	inline VectorT& operator*=(T rhs) {
		self().simd = SimdT::mul(self().simd, rhs);
		return self();
	}

	/// <summary> Scales the vector by 1/<paramref name="rhs"/>. </summary>
	inline VectorT& operator/=(T rhs) {
		self().simd = SimdT::div(self().simd, rhs);
		return self();
	}

	/// <summary> Adds <paramref name="rhs"/> to each element of the vector. </summary>
	inline VectorT& operator+=(T rhs) {
		self().simd = SimdT::add(self().simd, rhs);
		return self();
	}

	/// <summary> Subtracts <paramref name="rhs"/> from each element of the vector. </summary>
	inline VectorT& operator-=(T rhs) {
		self().simd = SimdT::sub(self().simd, rhs);
		return self();
	}
};


// Dynamic vector ops
template <class T, bool Packed>
class VectorOps<T, DYNAMIC, Packed, false>;
template <class T, bool Packed>
class VectorOps<T, DYNAMIC, Packed, true>;



//------------------------------------------------------------------------------
// Vector special operations
//------------------------------------------------------------------------------

// Note: VectorSpecialOps inherits from VectorOps instead of making
// Vector directly inherit from both because MSVC sucks hard with EBCO, and bloats
// Vector with useless bullshit twice the size it should be.

/// <summary> Implements cross product and other special operations. Don't use this class on its own. </summary>
template <class T, int Dim, bool Packed>
class VectorSpecialOps : public VectorOps<T, Dim, Packed> {
	using VectorT = Vector<T, Dim, Packed>;
public:
	/// <summary> Returns the generalized cross-product in N dimensions. </summary>
	/// <remarks> You must supply N-1 arguments of type Vector&lt;N&gt;.
	/// The function returns the generalized cross product as defined by
	/// https://en.wikipedia.org/wiki/Cross_product#Multilinear_algebra. </remarks>
	template <class... Args>
	static VectorT Cross(const VectorT& head, Args&&... args);

	/// <summary> Returns the generalized cross-product in N dimensions. </summary>
	/// <remarks> See https://en.wikipedia.org/wiki/Cross_product#Multilinear_algebra for definition. </remarks>
	static VectorT Cross(const std::array<const VectorT*, Dim - 1>& args);
};

/// <summary> Implements cross product and other special operations. Don't use this class on its own. </summary>
template <class T, bool Packed>
class VectorSpecialOps<T, 2, Packed> : public VectorOps<T, 2, Packed> {
	using VectorT = Vector<T, 2, Packed>;
public:
	/// <summary> Returns the 2-dimensional cross prodct, which is a vector perpendicular to the argument. </summary>
	static VectorT Cross(const VectorT& arg) {
		return VectorT(-arg.y,
					   arg.x);
	}
	/// <summary> Returns the 2-dimensional cross prodct, which is a vector perpendicular to the argument. </summary>
	static VectorT Cross(const std::array<const VectorT*, 1>& arg) {
		return Cross(*(arg[0]));
	}
};

/// <summary> Implements cross product and other special operations. Don't use this class on its own. </summary>
template <class T, bool Packed>
class VectorSpecialOps<T, 3, Packed> : public VectorOps<T, 3, Packed> {
	using VectorT = Vector<T, 3, Packed>;
public:
	/// <summary> Returns the 3-dimensional cross-product. </summary>
	static VectorT Cross(const VectorT& lhs, const VectorT& rhs) {
		return VectorT(lhs.y * rhs.z - lhs.z * rhs.y,
					   lhs.z * rhs.x - lhs.x * rhs.z,
					   lhs.x * rhs.y - lhs.y * rhs.x);
	}
	/// <summary> Returns the 3-dimensional cross-product. </summary>
	static VectorT Cross(const std::array<const VectorT*, 2>& args) {
		return Cross(*(args[0]), *(args[1]));
	}
};



//------------------------------------------------------------------------------
// General vector class
//------------------------------------------------------------------------------


/// <summary> Represents a vector in N-dimensional space. </summary>
/// <typeparam name="T"> The scalar type on which the vector is based.
///	You can use builtin floating point or integer types. User-defined types and std::complex
/// may also work, but are not yet officially supported. </typeparam>
/// <typeparam name="Dim"> The dimension of the vector-space. Must be a positive integer.
/// Dynamically sized vectors are not supported yet, but you'll have to use 
/// <see cref="mathter::DYNAMIC"/> to define dynamically sized vectors. </typeparam>
///	<typeparam name="Packed"> Set to true to tightly pack vector elements and
/// avoid padding of the vector struct. Disables SIMD optimizations. </typeparam>
/// <remarks>
/// There is not much extraordinary to vectors, they work as you would expect.
/// - you can use common vector space airhtmetic
/// - you have common function like normalization
/// - you can multiply them with <see cref="mathter::Matrix"/> from either side
/// - you can concatenate and swizzle them.
/// </remarks>
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
		os << "VectorData:       " << (intptr_t)static_cast<VectorData<T, 4, false>*>(ptr) - 1000 << " -> " << sizeof(VectorData<T, 4, false>) << std::endl;
		os << "VectorOps:        " << (intptr_t)static_cast<VectorOps<T, 4, false>*>(ptr) - 1000 << " -> " << sizeof(VectorOps<T, 4, false>) << std::endl;
		os << "VectorSpecialOps: " << (intptr_t)static_cast<VectorSpecialOps<T, 4, false>*>(ptr) - 1000 << " -> " << sizeof(VectorSpecialOps<T, 4, false>) << std::endl;
		os << "Vector:           " << (intptr_t)static_cast<Vector<T, 4, false>*>(ptr) - 1000 << " -> " << sizeof(Vector<T, 4, false>) << std::endl;
	}


	using VectorData<T, Dim, Packed>::data;

	//--------------------------------------------
	// Properties
	//--------------------------------------------
	constexpr int Dimension() const {
		return Dim;
	}

	//--------------------------------------------
	// Data constructors
	//--------------------------------------------

	/// <summary> Constructs the vector. Does NOT zero-initialize elements. </summary>
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


	/// <summary> Sets all elements to the same value. </summary>
	explicit Vector(T all) {
		CheckLayoutContraints();
		VectorOps<T, Dim, Packed>::spread(*this, all);
	}

	/// <summary> Constructs the vector from an array of elements. </summary>
	/// <remarks> The number of elements must be the same as the vector's dimension. </remarks>
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

	/// <summary> Creates a homogeneous vector by appending a 1. </summary>
	template <class T2, bool Packed2, class = typename std::enable_if<(Dim>=2), T2>::type>
	explicit Vector(const Vector<T2, Dim-1, Packed2>& rhs) : Vector(rhs, 1) {}

	/// <summary> Truncates last coordinate of homogenous vector to create non-homogeneous. </summary>
	template <class T2, bool Packed2>
	explicit Vector(const Vector<T2, Dim+1, Packed2>& rhs) : Vector(rhs.data) {}


	//--------------------------------------------
	// Copy, assignment, set
	//--------------------------------------------

	// NOTE: somehow msvc 2015 is buggy and cannot compile sizeof... checks for ctors as in Set

	/// <summary> Initializes the vector to the given scalar elements. </summary>
	/// <remarks> Number of arguments must equal vector dimension.
	///		Types of arguments may differ from vector's underlying type, in which case explicit cast is performed. </remarks>
	template <class H1, class H2, class... Scalars, typename std::enable_if<impl::All<impl::IsScalar, H1, H2, Scalars...>::value && impl::SumDimensions<H1, H2, Scalars...>::value == Dim, int>::type = 0>
	Vector(H1 h1, H2 h2, Scalars... scalars) {
		CheckLayoutContraints();
		Assign(0, h1, h2, scalars...);
	}

	/// <summary> Initializes the vector by concatenating given scalar and vector arguments. </summary>
	/// <remarks> Sum of the dimension of arguments must equal vector dimension. 
	///		Types of arguments may differ from vector's underlying type, in which case explicit cast is performed. </remarks>
	template <class H1, class... Mixed, typename std::enable_if<impl::Any<impl::IsVectorOrSwizzle, H1, Mixed...>::value && impl::SumDimensions<H1, Mixed...>::value == Dim, int>::type = 0>
	Vector(const H1& h1, const Mixed&... mixed) {
		CheckLayoutContraints();
		Assign(0, h1, mixed...);
	}

	/// <summary> Sets the vector's elements to the given scalars. </summary>
	/// <remarks> Number of arguments must equal vector dimension.
	///		Types of arguments may differ from vector's underlying type, in which case explicit cast is performed. </remarks>
	template <class... Scalars, typename std::enable_if<((sizeof...(Scalars) > 1) && impl::All<impl::IsScalar, Scalars...>::value), int>::type = 0>
	Vector& Set(Scalars... scalars) {
		static_assert(impl::SumDimensions<Scalars...>::value == Dim, "Arguments must match vector dimension.");
		Assign(0, scalars...);
		return *this;
	}

	/// <summary> Sets the vector's elements by concatenating given scalar and vector arguments. </summary>
	/// <remarks> Sum of the dimension of arguments must equal vector dimension. 
	///		Types of arguments may differ from vector's underlying type, in which case explicit cast is performed. </remarks>
	template <class... Mixed, typename std::enable_if<(sizeof...(Mixed) > 0) && impl::Any<impl::IsVectorOrSwizzle, Mixed...>::value, int>::type = 0>
	Vector& Set(const Mixed&... mixed) {
		static_assert(impl::SumDimensions<Mixed...>::value == Dim, "Arguments must match vector dimension.");
		Assign(0, mixed...);
		return *this;
	}

	/// <summary> Sets all elements of the vector to the given value. </summary>
	Vector& Spread(T all) {
		VectorOps<T, Dim, Packed>::spread(*this, all);

		return *this;
	}


	//--------------------------------------------
	// Accessors
	//--------------------------------------------

	/// <summary> Returns the nth element of the vector. </summary>
	T operator[](int idx) const {
		assert(idx < Dimension());
		return data[idx];
	}
	/// <summary> Returns the nth element of the vector. </summary>
	T& operator[](int idx) {
		assert(idx < Dimension());
		return data[idx];
	}

	/// <summary> Returns the nth element of the vector. </summary>
	T operator()(int idx) const {
		assert(idx < Dimension());
		return data[idx];
	}
	/// <summary> Returns the nth element of the vector. </summary>
	T& operator()(int idx) {
		assert(idx < Dimension());
		return data[idx];
	}

	/// <summary> Returns an iterator to the first element. </summary>
	const T* cbegin() const {
		return data;
	}
	/// <summary> Returns an iterator to the first element. </summary>
	const T* begin() const {
		return data;
	}
	/// <summary> Returns an iterator to the first element. </summary>
	T* begin() {
		return data;
	}
	/// <summary> Returns an iterator to the end of the vector (works like STL). </summary>
	const T* cend() const {
		return data + Dim;
	}
	/// <summary> Returns an iterator to the end of the vector (works like STL). </summary>
	const T* end() const {
		return data + Dim;
	}
	/// <summary> Returns an iterator to the end of the vector (works like STL). </summary>
	T* end() {
		return data + Dim;
	}

	/// <summary> Returns a pointer to the underlying array of elements. </summary>
	const T* Data() const {
		return data;
	}
	/// <summary> Returns a pointer to the underlying array of elements. </summary>
	T* Data() {
		return data;
	}


	//--------------------------------------------
	// Compare
	//--------------------------------------------

	/// <summary> Exactly compares two vectors. </summary>
	/// <remarks> &lt;The usual warning about floating point numbers&gt; </remarks>
	bool operator==(const Vector& rhs) const {
		bool same = data[0] == rhs.data[0];
		for (int i = 1; i < Dim; ++i) {
			same = same && data[i] == rhs.data[i];
		}
		return same;
	}

	/// <summary> Exactly compares two vectors. </summary>
	/// <remarks> &lt;The usual warning about floating point numbers&gt; </remarks>
	bool operator!=(const Vector& rhs) const {
		return !operator==(rhs);
	}

	/// <summary> Returns true if the elements of the vector are relatively close to each other. </summary>
	/// <remarks> Use for floating point numbers. 
	///		Note that this is not well-defined and is mostly used for unit tests
	///		and debugging. As such, you should not rely on it, and should rather implement
	///		one for your specific needs. </remarks>
	bool AlmostEqual(const Vector& rhs) const {
		bool same = true;
		for (int i = 0; i < Dim; ++i) {
			T d1 = data[i], d2 = rhs.data[i];
			bool memberEqual = impl::AlmostEqual(d1, d2);
			same = same && memberEqual;
		}
		return same;
	}
	auto Approx() const {
		return mathter::ApproxHelper<Vector>(*this);
	}

	//--------------------------------------------
	// Arithmetic
	//--------------------------------------------


	/// <summary> Negates all elements of the vector. </summary>
	inline Vector operator-() const {
		return (*this) * T(-1);
	}

	/// <summary> Optional plus sign, leaves the vector as is. </summary>
	inline Vector operator+() const {
		return *this;
	}

	// Vector arithmetic
	using VectorOps<T, Dim, Packed>::operator*;
	using VectorOps<T, Dim, Packed>::operator/;
	using VectorOps<T, Dim, Packed>::operator+;
	using VectorOps<T, Dim, Packed>::operator-;

	using VectorOps<T, Dim, Packed>::operator*=;
	using VectorOps<T, Dim, Packed>::operator/=;
	using VectorOps<T, Dim, Packed>::operator+=;
	using VectorOps<T, Dim, Packed>::operator-=;


	// Scalar arithmetic
	/// <summary> Scales the vector by <paramref name="rhs"/>. </summary>
	inline Vector operator*(T rhs) const { return Vector(*this) *= rhs; }
	/// <summary> Scales the vector by 1/<paramref name="rhs"/>. </summary>
	inline Vector operator/(T rhs) const { return Vector(*this) /= rhs; }
	/// <summary> Adds <paramref name="rhs"/> to each element of the vector. </summary>
	inline Vector operator+(T rhs) const { return Vector(*this) += rhs; }
	/// <summary> Subtracts <paramref name="rhs"/> from each element of the vector. </summary>
	inline Vector operator-(T rhs) const { return Vector(*this) -= rhs; }

	/// <summary> Return (a*b)+c. Performs MAD or FMA if supported by target architecture. </summary>
	static inline Vector MultiplyAdd(const Vector& a, const Vector& b, const Vector& c) {
		return fma(a, b, c);
	}


	//--------------------------------------------
	// Common functions
	//--------------------------------------------

	/// <summary> Makes a unit vector, but keeps direction. </summary> 
	void Normalize() {
		assert(!IsNullvector());
		T l = Length();
		operator/=(l);
	}

	/// <summary> Returns the unit vector having the same direction, without modifying the object. </summary>
	Vector Normalized() const {
		assert(!IsNullvector());
		Vector v = *this;
		v.Normalize();
		return v;
	}

	/// <summary> Checks if the vector is unit vector. There's some tolerance due to floating points. </summary>
	bool IsNormalized() const {
		T n = LengthSquared();
		return T(0.9999) <= n && n <= T(1.0001);
	}

	/// <summary> Makes a unit vector, but keeps direction. Leans towards (1,0,0...) for nullvectors, costs more. </summary>
	void SafeNormalize() {
		(*this)(0) = std::abs((*this)(0)) > std::numeric_limits<T>::denorm_min() ? (*this)(0) : std::numeric_limits<T>::denorm_min();
		T l = LengthPrecise();
		operator/=(l);
	}

	/// <summary> Returns the unit vector having the same direction, without modifying the object. Leans towards (1,0,0...) for nullvectors, costs more. </summary>
	Vector SafeNormalized() const {
		Vector v = *this;
		v.SafeNormalize();
		return v;
	}


	/// <summary> Returns true if the vector's length is too small for precise calculations (i.e. normalization). </summary>
	/// <remarks> "Too small" means smaller than the square root of the smallest number representable by the underlying scalar. 
	///			This value is ~10^-18 for floats and ~10^-154 for doubles. </remarks>
	bool IsNullvector() const {
		static constexpr T epsilon = T(1) / impl::ConstexprExp10<T>(impl::ConstexprAbs(std::numeric_limits<T>::min_exponent10) / 2);
		T length = Length();
		return length < epsilon;
	}


	/// <summary> Returns the scalar product of the two vectors. </summary>
	static T Dot(const Vector& lhs, const Vector& rhs) {
		return Vector::dot(lhs, rhs);
	}

	/// <summary> Returns the scalar product of the two vectors. </summary>
	template <class T2, bool Packed2, typename std::enable_if<!std::is_same<T, T2>::value, int>::type = 0>
	static auto Dot(const Vector& lhs, const Vector<T2, Dim, Packed2>& rhs) {
		auto s = lhs.data[0] * rhs.data[0];
		for (int i = 1; i < Dim; ++i) {
			s = lhs.data[i] * rhs.data[i] + s;
		}
		return s;
	}

	/// <summary> Returns the squared length of the vector. </summary>
	T LengthSquared() const {
		return Dot(*this, *this);
	}

	/// <summary> Returns the length of the vector. </summary> 
	T Length() const {
		return (T)sqrt((T)LengthSquared());
	}

	/// <summary> Returns the length of the vector, avoids overflow and underflow, so it's more expensive. </summary>
	T LengthPrecise() const {
		T maxElement = std::abs((*this)(0));
		for (int i = 1; i<Dimension(); ++i) {
			maxElement = std::max(maxElement, std::abs((*this)(i)));
		}
		if (maxElement == T(0)) {
			return T(0);
		}
		auto scaled = (*this)/maxElement;
		return sqrt(Dot(scaled, scaled))*maxElement;
	}


	/// <summary> Returns the element-wise minimum of arguments </summary>
	static Vector Min(const Vector& lhs, const Vector& rhs) {
		Vector res;
		for (int i = 0; i < lhs.Dimension(); ++i) {
			res[i] = std::min(lhs[i], rhs[i]);
		}
		return res;
	}
	/// <summary> Returns the element-wise maximum of arguments </summary>
	static Vector Max(const Vector& lhs, const Vector& rhs) {
		Vector res;
		for (int i = 0; i < lhs.Dimension(); ++i) {
			res[i] = std::max(lhs[i], rhs[i]);
		}
		return res;
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
	template <class T2, int D2, bool P2>
	struct GetVectorElement<Vector<T2, D2, P2>> {
		static T2 Get(const Vector<T2, D2, P2>& u, int idx) { return u.data[idx]; }
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
			data[idx] = (T)GetVectorElement<Head>::Get(head, i);
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

/// <summary> Scales vector by <paramref name="lhs"/>. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator*(U lhs, const Vector<T, Dim, Packed>& rhs) {
	return rhs*(T)lhs;
}

/// <summary> Adds <paramref name="lhs"/> to all elements of the vector. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator+(U lhs, const Vector<T, Dim, Packed>& rhs) {
	return rhs + (T)lhs;
}

/// <summary> Makes a vector with <paramref name="lhs"/> as all elements, then subtracts <paramref name="rhs"> from it. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator-(U lhs, const Vector<T, Dim, Packed>& rhs) {
	return Vector<T, Dim, Packed>(lhs) - rhs;
}

/// <summary> Makes a vector with <paramref name="lhs"/> as all elements, then divides it by <paramref name="rhs">. </summary>
template <class T, int Dim, bool Packed, class U, class = typename std::enable_if<std::is_convertible<U, T>::value>::type>
inline Vector<T, Dim, Packed> operator/(U lhs, const Vector<T, Dim, Packed>& rhs) {
	Vector<T, Dim, Packed> res(lhs);
	res /= rhs;
	return res;
}



//------------------------------------------------------------------------------
// Common vector operations as globals
//------------------------------------------------------------------------------

/// <summary> Returns the scalar product. </summary>
template <class T, class U, int Dim, bool Packed1, bool Packed2>
auto Dot(const Vector<T, Dim, Packed1>& lhs, const Vector<U, Dim, Packed2>& rhs) {
	return Vector<T, Dim, Packed1>::Dot(lhs, rhs);
}


/// <summary> Returns the n-dimensional generalized cross product. </summary>
template <class T, int Dim, bool Packed, class... Vectors>
auto Cross(const Vector<T, Dim, Packed>& head, const Vectors&... vectors) {
	return Vector<T, Dim, Packed>::Cross(head, vectors...);
}

/// <summary> Returns the n-dimensional generalized cross product. </summary>
template <class T, int Dim, bool Packed>
auto Cross(const std::array<const Vector<T, Dim, Packed>*, Dim - 1>& vectors) {
	return Vector<T, Dim, Packed>::Cross(vectors);
}


/// <summary> Returns the euclidean distance between to vectors. </summary>
template <class T, class U, int Dim, bool Packed1, bool Packed2>
auto Distance(const Vector<T, Dim, Packed1>& lhs, const Vector<U, Dim, Packed2>& rhs) {
	return (lhs - rhs).Length();
}

/// <summary> Returns the normalized version of <paramref name="arg">. </summary>
template <class T, int Dim, bool Packed>
auto Normalized(const Vector<T, Dim, Packed>& arg) {
	return arg.Normalized();
}

/// <summary> Return the elementwise minimum of the arguments. </summary>
template <class T, int Dim, bool Packed>
auto Min(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	return Vector<T, Dim, Packed>::Min(lhs, rhs);
}
/// <summary> Return the elementwise maximum of the arguments. </summary>
template <class T, int Dim, bool Packed>
auto Max(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	return Vector<T, Dim, Packed>::Max(lhs, rhs);
}


//------------------------------------------------------------------------------
// Vector concatenation
//------------------------------------------------------------------------------

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T, int Dim, bool Packed, class U>
mathter::Vector<T, Dim + 1, Packed> operator|(const mathter::Vector<T, Dim, Packed>& lhs, U rhs) {
	mathter::Vector<T, Dim + 1, Packed> ret;
	for (int i = 0; i < Dim; ++i) {
		ret(i) = lhs(i);
	}
	ret(Dim) = rhs;
	return ret;
}

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
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

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T, int Dim, bool Packed, class U>
mathter::Vector<T, Dim + 1, Packed> operator|(U lhs, const mathter::Vector<T, Dim, Packed>& rhs) {
	mathter::Vector<T, Dim + 1, Packed> ret;
	ret(0) = lhs;
	for (int i = 0; i < Dim; ++i) {
		ret(i + 1) = rhs(i);
	}
	return ret;
}

/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class T2, int... Indices2>
Vector<T1, sizeof...(Indices2)+sizeof...(Indices2), false> operator|(const Swizzle<T1, Indices1...>& lhs, const Swizzle<T2, Indices2...>& rhs) {
	return Vector<T1, sizeof...(Indices1), false>(lhs) | Vector<T1, sizeof...(Indices2), false>(rhs);
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class T2, int Dim, bool Packed>
Vector<T1, sizeof...(Indices1)+Dim, Packed> operator|(const Swizzle<T1, Indices1...>& lhs, const Vector<T2, Dim, Packed>& rhs) {
	return Vector<T1, sizeof...(Indices1), Packed>(lhs) | rhs;
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class T2, int Dim, bool Packed>
Vector<T1, sizeof...(Indices1)+Dim, Packed> operator|(const Vector<T2, Dim, Packed>& lhs, const Swizzle<T1, Indices1...>& rhs) {
	return lhs | Vector<T1, sizeof...(Indices1), false>(rhs);
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class U>
Vector<T1, sizeof...(Indices1)+1, false> operator|(const Swizzle<T1, Indices1...>& lhs, U rhs) {
	return Vector<T1, sizeof...(Indices1), false>(lhs) | rhs;
}
/// <summary> Concatenates the arguments, and returns the concatenated vector. </summary>
template <class T1, int... Indices1, class U>
Vector<T1, sizeof...(Indices1)+1, false> operator|(U lhs, const Swizzle<T1, Indices1...>& rhs) {
	return lhs | Vector<T1, sizeof...(Indices1), false>(rhs);
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



//------------------------------------------------------------------------------
// IO
//------------------------------------------------------------------------------

enum class eEnclosingBracket {
	NONE,
	PARANTHESE,
	BRACKET,
	BRACE,
};

/// <summary> Prints the vector like [1,2,3]. </summary>
template <class T, int Dim, bool Packed>
std::ostream& operator<<(std::ostream& os, const mathter::Vector<T, Dim, Packed>& v) {
	os << "[";
	for (int x = 0; x < Dim; ++x) {
		os << v(x) << (x == Dim - 1 ? "" : ", ");
	}
	os << "]";
	return os;
}


namespace impl {
	template <class T>
	struct dependent_false {
		static constexpr bool value = false;
	};
	template <class T>
	constexpr bool dependent_false_v = dependent_false<T>::value;

	template <class AritT, typename std::enable_if<std::is_integral<AritT>::value && std::is_signed<AritT>::value, int>::type = 0>
	AritT strtonum(const char* str, const char** end) {
		AritT value;
		value = (AritT)strtoll(str, (char**)end, 10);
		return value;
	}
	template <class AritT, typename std::enable_if<std::is_integral<AritT>::value && !std::is_signed<AritT>::value, int>::type = 0>
	AritT strtonum(const char* str, const char** end) {
		AritT value;
		value = (AritT)strtoull(str, (char**)end, 10);
		return value;
	}
	template <class AritT, typename std::enable_if<std::is_floating_point<AritT>::value, int>::type = 0>
	AritT strtonum(const char* str, const char** end) {
		AritT value;
		value = (AritT)strtold(str, (char**)end);
		return value;
	}

	inline const char* StripSpaces(const char* str) {
		while (*str != '\0' && isspace(*str))
			++str;
		return str;
	};

} // namespace impl

/// <summary> Parses a vector from a string. </summary>
template <class T, int Dim, bool Packed>
Vector<T, Dim, Packed> strtovec(const char* str, const char** end) {
	Vector<T, Dim, Packed> ret;

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

	// parse elements
	for (int i = 0; i < Dim; ++i) {
		const char* elemend;
		T elem = impl::strtonum<T>(strproc, &elemend);
		if (elemend == strproc) {
			*end = str;
			return ret;
		}
		else {
			ret[i] = elem;
			strproc = elemend;
		}
		strproc = impl::StripSpaces(strproc);
		if (*strproc == ',') {
			++strproc;
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

template <class VectorT>
VectorT strtovec(const char* str, const char** end) {
	static_assert(impl::IsVector<VectorT>::value, "This type is not a Vector, dumbass.");

	return strtovec<
		typename impl::VectorProperties<VectorT>::Type,
		impl::VectorProperties<VectorT>::Dim,
		impl::VectorProperties<VectorT>::Packed>
		(str, end);
}


} // namespace mathter



// Generalized cross-product unfortunately needs matrix determinant.
#include "Matrix.hpp"

namespace mathter {

template<class T, int Dim, bool Packed>
auto VectorSpecialOps<T, Dim, Packed>::Cross(const std::array<const VectorT *, Dim - 1> &args) -> VectorT {
	VectorT result;
	Matrix<T, Dim - 1, Dim - 1, eMatrixOrder::FOLLOW_VECTOR, eMatrixLayout::ROW_MAJOR, false> detCalc;

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


template<class T, int Dim, bool Packed>
template<class... Args>
auto VectorSpecialOps<T, Dim, Packed>::Cross(const VectorT &head, Args &&... args) -> VectorT {
	static_assert(1 + sizeof...(args) == Dim - 1, "Number of arguments must be (Dimension - 1).");

	std::array<const VectorT *, Dim - 1> vectors = {&head, &args...};
	return Cross(vectors);
}

} // namespace mathter


  // Remove goddamn fucking bullshit crapware winapi macros.
#if defined(MATHTER_MINMAX)
#pragma pop_macro("min")
#pragma pop_macro("max")
#endif
