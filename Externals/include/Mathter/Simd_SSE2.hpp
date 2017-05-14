//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include <emmintrin.h>


// Specialization for float4, using SSE
template <>
union alignas(16) Simd<float, 4> {
	__m128 reg;
	float v[4];

	static inline Simd mul(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_mul_ps(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd div(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_div_ps(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd add(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_add_ps(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd sub(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_sub_ps(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd mul(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg = _mm_mul_ps(lhs.reg, tmp);
		return res;
	}

	static inline Simd div(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg = _mm_div_ps(lhs.reg, tmp);
		return res;
	}

	static inline Simd add(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg = _mm_add_ps(lhs.reg, tmp);
		return res;
	}

	static inline Simd sub(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg = _mm_sub_ps(lhs.reg, tmp);
		return res;
	}

	static inline Simd mad(const Simd& a, const Simd& b, const Simd& c) {
		return add(mul(a, b), c);
	}

	static inline Simd spread(float value) {
		Simd res;
		res.reg = _mm_set1_ps(value);
		return res;
	}

	static inline Simd set(float x, float y, float z, float w) {
		Simd res;
		res.reg = _mm_setr_ps(x, y, z, w);
		return res;
	}

	template <int Count>
	static inline float dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= 4, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		float sum;
		Simd m = mul(lhs, rhs);
		sum = m.v[0];
		for (int i = 1; i < Count; ++i) {
			sum += m.v[i];
		}
		return sum;
	}
};



// Specialization for float8, using SSE
template <>
union alignas(16) Simd<float, 8> {
	__m128 reg[2];
	float v[4];


	static inline Simd mul(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_mul_ps(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd div(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_div_ps(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_div_ps(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd add(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_add_ps(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_add_ps(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd sub(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_sub_ps(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_sub_ps(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd mul(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg[0] = _mm_mul_ps(lhs.reg[0], tmp);
		res.reg[1] = _mm_mul_ps(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd div(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg[0] = _mm_div_ps(lhs.reg[0], tmp);
		res.reg[1] = _mm_div_ps(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd add(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg[0] = _mm_add_ps(lhs.reg[0], tmp);
		res.reg[1] = _mm_add_ps(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd sub(const Simd& lhs, float rhs) {
		Simd res;
		__m128 tmp = _mm_set1_ps(rhs);
		res.reg[0] = _mm_sub_ps(lhs.reg[0], tmp);
		res.reg[1] = _mm_sub_ps(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd mad(const Simd& a, const Simd& b, const Simd& c) {
		return add(mul(a, b), c);
	}

	static inline Simd spread(float value) {
		Simd res;
		res.reg[0] = _mm_set1_ps(value);
		res.reg[1] = _mm_set1_ps(value);
		return res;
	}

	static inline Simd set(float a, float b, float c, float d, float e, float f, float g, float h) {
		Simd res;
		res.reg[0] = _mm_setr_ps(a, b, c, d);
		res.reg[0] = _mm_setr_ps(e, f, g, h);
		return res;
	}


	template <int Count>
	static inline float dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= 8, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		__m128 reg1, reg2;
		reg1 = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
		reg2 = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);

		for (int i = Dim - 1; i >= Count && i >= 4; ++i) {
			reg2.m128_f32[i] = 0.0f;
		}
		for (int i = 3; i >= Count && i >= 0; ++i) {
			reg1.m128_f32[i] = 0.0f;
		}

		float sum;
		reg1 = _mm_add_ps(reg1, reg2);
		sum = reg1.m128_f32[0] + reg1.m128_f32[1] + reg1.m128_f32[2] + reg1.m128_f32[3];

		return sum;
	}
};