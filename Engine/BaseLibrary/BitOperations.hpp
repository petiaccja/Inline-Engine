#pragma once

#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)

#endif


namespace exc {


//------------------------------------------------------------------------------
// CTZ, CLZ
//------------------------------------------------------------------------------


/// <summary> Counts the trailing zero bits of a 32 bit unsigned argument. From LSB to MSB. </summary>
/// <returns> If the argument is zero, -1 is returned, the number of trailing zeros otherwise. </returns> 
inline int CountTrailingZeros(uint32_t arg) {
#if defined(_MSC_VER)
	unsigned long index;
	uint8_t res = _BitScanForward(&index, arg);
	return res > 0 ? (int)index : -1;
#elif defined(__GNUC__)
	return arg == 0 ? -1 : __builtin_ctz(arg);
#else 
	for (uint8_t idx = 0; idx < 32; ++idx) {
		if ((arg >> idx) & 0x01)
			return idx;
	}
	return -1;
#endif
}

/// <summary> Counts the leading zero bits of a 32 bit unsigned argument. From MSB to LSB. </summary>
/// <returns> If the argument is zero, -1 is returned, the number of leading zeros otherwise. </returns> 
inline int CountLeadingZeros(uint32_t arg) {
#if defined(_MSC_VER)
	unsigned long index;
	uint8_t res = _BitScanReverse(&index, arg);
	return res > 0 ? (31 - (int)index) : -1;
#elif defined(__GNUC__)
	return arg == 0 ? -1 : __builtin_clz(arg);
#else 
	for (uint8_t idx = 0; idx < 32; ++idx) {
		if ((arg << idx) & 0x8000'0000U)
			return idx;
	}
	return -1;
#endif
}

/// <summary> Counts the trailing zero bits of a 64 bit unsigned argument. From LSB to MSB. </summary>
/// <returns> If the argument is zero, -1 is returned, the number of trailing zeros otherwise. </returns> 
inline int CountTrailingZeros(uint64_t arg) {
#if defined(_MSC_VER)
	unsigned long index;
	uint8_t res = _BitScanForward64(&index, arg);
	return res > 0 ? (int)index : -1;
#elif defined(__GNUC__)
	return arg == 0 ? -1 : __builtin_ctzll(arg);
#else 
	for (uint8_t idx = 0; idx < 64; ++idx) {
		if ((arg >> idx) & 0x01)
			return idx;
	}
	return -1;
#endif
}

/// <summary> Counts the leading zero bits of a 64 bit unsigned argument. From MSB to LSB. </summary>
/// <returns> If the argument is zero, -1 is returned, the number of leading zeros otherwise. </returns> 
inline int CountLeadingZeros(uint64_t arg) {
#if defined(_MSC_VER)
	unsigned long index;
	uint8_t res = _BitScanReverse64(&index, arg);
	return res > 0 ? (31 - (int)index) : -1;
#elif defined(__GNUC__)
	return arg == 0 ? -1 : __builtin_clzll(arg);
#else 
	for (uint8_t idx = 0; idx < 64; ++idx) {
		if ((arg << idx) & 0x8000'0000'0000'0000ULL)
			return idx;
	}
	return -1;
#endif
}


//------------------------------------------------------------------------------
// Bit clear/set
//------------------------------------------------------------------------------

inline bool BitTestAndClear(uint32_t& arg, unsigned bit) {
#if defined(_MSC_VER)
	return 0 != _bittestandreset(reinterpret_cast<long*>(&arg), bit);
#else
	bool ret = arg & (1u << bit);
	arg &= ~(1u << bit);
	return ret;
#endif
}


inline bool BitTestAndSet(uint32_t& arg, unsigned bit) {
#if defined(_MSC_VER)
	return 0 != _bittestandset(reinterpret_cast<long*>(&arg), bit);
#else
	bool ret = arg & (1u << bit);
	arg |= (1u << bit);
	return ret;
#endif
}


inline bool BitTestAndClear(uint64_t& arg, unsigned bit) {
#if defined(_MSC_VER)
	return 0 != _bittestandreset64(reinterpret_cast<long long*>(&arg), bit);
#else
	bool ret = arg & (1ull << bit);
	arg &= ~(1ull << bit);
	return ret;
#endif
}


inline bool BitTestAndSet(uint64_t& arg, unsigned bit) {
#if defined(_MSC_VER)
	return 0 != _bittestandset64(reinterpret_cast<long long*>(&arg), bit);
#else
	bool ret = arg & (1ull << bit);
	arg |= (1ull << bit);
	return ret;
#endif
}


//------------------------------------------------------------------------------
// Count set bits
//------------------------------------------------------------------------------

// TODO...




} // namespace exc