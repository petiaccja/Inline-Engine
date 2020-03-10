//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include <cstdint>
#include <cassert>


// Default implementation without SSE.
// Additional implementation are #included as template specializations of this class.

/// <summary>
/// 2,4 or 8 dimension float or double parameters accepted.
/// Uses SSE2 or AVX acceleration if enabled in the compiler.
/// </summary>
template <class T, int Dim>
union Simd {
	static_assert(Dim == 2 || Dim == 4 || Dim == 8, "Dimension must be 2, 4, or 8.");
	static_assert(std::is_same<T, float>::value 
				  || std::is_same<T, double>::value
				  || std::is_same<T, int32_t>::value
				  || std::is_same<T, int64_t>::value,
				  "Type must be float, double, in32 or int64.");
public:
	T v[Dim];

	static inline Simd mul(const Simd& lhs, const Simd& rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] * rhs.v[i];
		return res;
	}

	static inline Simd div(const Simd& lhs, const Simd& rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] / rhs.v[i];
		return res;
	}

	static inline Simd add(const Simd& lhs, const Simd& rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] + rhs.v[i];
		return res;
	}

	static inline Simd sub(const Simd& lhs, const Simd& rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] - rhs.v[i];
		return res;
	}

	static inline Simd mul(const Simd& lhs, T rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] * rhs;
		return res;
	}

	static inline Simd div(const Simd& lhs, T rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] / rhs;
		return res;
	}

	static inline Simd add(const Simd& lhs, T rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] + rhs;
		return res;
	}

	static inline Simd sub(const Simd& lhs, T rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] - rhs;
		return res;
	}

	static inline Simd mad(const Simd& a, const Simd& b, const Simd& c) {
		return add(mul(a, b), c);
	}

	static inline Simd spread(T value) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = value;
		return res;
	}

	template <class... Args>
	static inline Simd set(Args... args) {
		Simd res;
		static_assert(sizeof...(Args) == Dim, "Number of arguments must be equal to dimension.");
		T table[] = { T(args)... };
		for (int i = 0; i < Dim; ++i)
			res.v[i] = table[i];
		return res;
	}

	template <int Count = Dim>
	static inline T dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= Dim, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		T sum = lhs.v[0] * rhs.v[0];
		for (int i = 1; i < Count; ++i)
			sum += lhs.v[i] * rhs.v[i];
		return sum;
	}

	template <int i0, int i1>
	static inline Simd shuffle(Simd arg) {
		static_assert(Dim == 2, "Only for 2-way simd.");
		Simd ret;
		ret.v[1] = arg.v[i0];
		ret.v[0] = arg.v[i1];
		return ret;
	}

	template <int i0, int i1, int i2, int i3>
	static inline Simd shuffle(Simd arg) {
		static_assert(Dim == 4, "Only for 4-way simd.");
		Simd ret;
		ret.v[3] = arg.v[i0];
		ret.v[2] = arg.v[i1];
		ret.v[1] = arg.v[i2];
		ret.v[0] = arg.v[i3];
		return ret;
	}

	template <int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7>
	static inline Simd shuffle(Simd arg) {
		static_assert(Dim == 8, "Only for 8-way simd.");
		Simd ret;
		ret.v[7] = arg.v[i0];
		ret.v[6] = arg.v[i1];
		ret.v[5] = arg.v[i2];
		ret.v[4] = arg.v[i3];
		ret.v[3] = arg.v[i4];
		ret.v[2] = arg.v[i5];
		ret.v[1] = arg.v[i6];
		ret.v[0] = arg.v[i7];
		return ret;
	}
};




#if defined(__SSE2__) || _M_IX86_FP >= 2 || _M_X64 

#include "Simd_SSE2.hpp"

#endif