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
	static_assert(std::is_same<T, float>::value || std::is_same<T, double>::value, "Type must be float or double.");
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

	static inline Simd mul(const Simd& lhs, float rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] * rhs;
		return res;
	}

	static inline Simd div(const Simd& lhs, float rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] / rhs;
		return res;
	}

	static inline Simd add(const Simd& lhs, float rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] + rhs;
		return res;
	}

	static inline Simd sub(const Simd& lhs, float rhs) {
		Simd res;
		for (int i = 0; i < Dim; ++i)
			res.v[i] = lhs.v[i] - rhs;
		return res;
	}

	static inline Simd mad(const Simd& a, const Simd& b, const Simd& c) {
		return add(mul(a, b), c);
	}

	static inline Simd spread(float value) {
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
	static inline float dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= Dim, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		float sum = lhs.v[0] * rhs.v[0];
		for (int i = 1; i < Count; ++i)
			sum += lhs.v[i] * rhs.v[i];
		return sum;
	}

};




#if defined(__SSE2__) || _M_IX86_FP >= 2 || _M_X64 

#include "Simd_SSE2.hpp"

#endif