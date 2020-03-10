#pragma once

#include "VectorImpl.hpp"

namespace mathter {

/// <summary> Exactly compares two vectors. </summary>
/// <remarks> &lt;The usual warning about floating point numbers&gt; </remarks>
template <class T, int Dim, bool Packed>
bool operator==(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	bool same = lhs.data[0] == rhs.data[0];
	for (int i = 1; i < Dim; ++i) {
		same = same && lhs.data[i] == rhs.data[i];
	}
	return same;
}

/// <summary> Exactly compares two vectors. </summary>
/// <remarks> &lt;The usual warning about floating point numbers&gt; </remarks>
template <class T, int Dim, bool Packed>
bool operator!=(const Vector<T, Dim, Packed>& lhs, const Vector<T, Dim, Packed>& rhs) {
	return !operator==(rhs);
}

} // namespace mathter