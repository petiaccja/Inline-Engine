#pragma once


namespace inl {

inline size_t CombineHash(size_t hash1, size_t hash2) {
	return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}

} // namespace inl