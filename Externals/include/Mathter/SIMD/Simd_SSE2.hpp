//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include <emmintrin.h>


//------------------------------------------------------------------------------
// FLOAT
//------------------------------------------------------------------------------

// Specialization for float4, using SSE
template <>
union alignas(16) Simd<float, 4> {
	__m128 reg;
	__m128i regi;
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

	template <int i0, int i1, int i2, int i3>
	static inline Simd shuffle(const Simd& arg) {
		Simd ret;
		ret.regi = _mm_shuffle_epi32(arg.regi, _MM_SHUFFLE(i0, i1, i2, i3));
		return ret;
	}
};



// Specialization for float8, using SSE
template <>
union alignas(16) Simd<float, 8> {
	__m128 reg[2];
	float v[8];


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
		res.reg[1] = _mm_setr_ps(e, f, g, h);
		return res;
	}


	template <int Count>
	static inline float dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= 8, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		__m128 reg1, reg2;
		reg1 = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
		reg2 = _mm_mul_ps(lhs.reg[1], rhs.reg[1]);

		for (int i = 7; i >= Count && i >= 4; --i) {
			reinterpret_cast<float*>(&reg2)[i] = 0.0f;
		}
		for (int i = 3; i >= Count && i >= 0; --i) {
			reinterpret_cast<float*>(&reg1)[i] = 0.0f;
		}

		float sum;
		reg1 = _mm_add_ps(reg1, reg2);
		sum = reinterpret_cast<float*>(&reg1)[0]
			  + reinterpret_cast<float*>(&reg1)[1]
			  + reinterpret_cast<float*>(&reg1)[2]
			  + reinterpret_cast<float*>(&reg1)[3];

		return sum;
	}


	template <int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7>
	static inline Simd shuffle(const Simd& arg) {
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


//------------------------------------------------------------------------------
// DOUBLE
//------------------------------------------------------------------------------



// Specialization for double2, using SSE
template <>
union alignas(16) Simd<double, 2> {
	__m128d reg;
	double v[4];


	static inline Simd mul(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_mul_pd(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd div(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_div_pd(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd add(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_add_pd(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd sub(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg = _mm_sub_pd(lhs.reg, rhs.reg);
		return res;
	}

	static inline Simd mul(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg = _mm_mul_pd(lhs.reg, tmp);
		return res;
	}

	static inline Simd div(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg = _mm_div_pd(lhs.reg, tmp);
		return res;
	}

	static inline Simd add(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg = _mm_add_pd(lhs.reg, tmp);
		return res;
	}

	static inline Simd sub(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg = _mm_sub_pd(lhs.reg, tmp);
		return res;
	}

	static inline Simd mad(const Simd& a, const Simd& b, const Simd& c) {
		return add(mul(a, b), c);
	}

	static inline Simd spread(double value) {
		Simd res;
		res.reg = _mm_set1_pd(value);
		return res;
	}

	static inline Simd set(double x, double y) {
		Simd res;
		res.reg = _mm_setr_pd(x, y);
		return res;
	}

	template <int Count>
	static inline double dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= 2, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		double sum;
		Simd m = mul(lhs, rhs);
		sum = m.v[0];
		for (int i = 1; i < Count; ++i) {
			sum += m.v[i];
		}
		return sum;
	}

	template <int i0, int i1>
	static inline Simd shuffle(const Simd& arg) {
		Simd ret;
		ret.reg = _mm_shuffle_pd(arg.reg, arg.reg, _MM_SHUFFLE2(i0, i1));
		return ret;
	}
};



// Specialization for double4, using SSE
//*
template <>
union alignas(16) Simd<double, 4> {
	__m128d reg[2];
	double v[4];


	static inline Simd mul(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_mul_pd(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_mul_pd(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd div(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_div_pd(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_div_pd(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd add(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_add_pd(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_add_pd(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd sub(const Simd& lhs, const Simd& rhs) {
		Simd res;
		res.reg[0] = _mm_sub_pd(lhs.reg[0], rhs.reg[0]);
		res.reg[1] = _mm_sub_pd(lhs.reg[1], rhs.reg[1]);
		return res;
	}

	static inline Simd mul(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg[0] = _mm_mul_pd(lhs.reg[0], tmp);
		res.reg[1] = _mm_mul_pd(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd div(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg[0] = _mm_div_pd(lhs.reg[0], tmp);
		res.reg[1] = _mm_div_pd(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd add(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg[0] = _mm_add_pd(lhs.reg[0], tmp);
		res.reg[1] = _mm_add_pd(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd sub(const Simd& lhs, double rhs) {
		Simd res;
		__m128d tmp = _mm_set1_pd(rhs);
		res.reg[0] = _mm_sub_pd(lhs.reg[0], tmp);
		res.reg[1] = _mm_sub_pd(lhs.reg[1], tmp);
		return res;
	}

	static inline Simd mad(const Simd& a, const Simd& b, const Simd& c) {
		return add(mul(a, b), c);
	}

	static inline Simd spread(double value) {
		Simd res;
		res.reg[0] = _mm_set1_pd(value);
		res.reg[1] = _mm_set1_pd(value);
		return res;
	}

	static inline Simd set(double x, double y, double z, double w) {
		Simd res;
		res.reg[0] = _mm_setr_pd(x, y);
		res.reg[1] = _mm_setr_pd(z, w);
		return res;
	}


	template <int Count>
	static inline double dot(const Simd& lhs, const Simd& rhs) {
		static_assert(Count <= 4, "Number of elements to dot must be smaller or equal to dimension.");
		static_assert(0 < Count, "Count must not be zero.");
		__m128d regs[2];
		regs[0] = _mm_mul_pd(lhs.reg[0], rhs.reg[0]);
		regs[1] = _mm_mul_pd(lhs.reg[1], rhs.reg[1]);

		for (int i = 3; i >= Count; --i) {
			reinterpret_cast<double*>(&regs)[i] = 0.0;
		}

		double sum;
		regs[0] = _mm_add_pd(regs[0], regs[1]);
		sum = reinterpret_cast<double*>(&regs[0])[0] + reinterpret_cast<double*>(&regs[0])[1];

		return sum;
	}


	template <int i0, int i1, int i2, int i3>
	static inline Simd shuffle(const Simd& arg) {
		Simd ret;
		ret.v[3] = arg.v[i0];
		ret.v[2] = arg.v[i1];
		ret.v[1] = arg.v[i2];
		ret.v[0] = arg.v[i3];
		return ret;
	}
};
//*/