// This file contains type aliases for most common linear algebra constructs.
// Mathfu types are used as a base.
// THIS IS NOT PART OF MATHFU LIBRARY! DO NOT REMOVE THIS FILE!

#pragma once

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

#include "vector_2.h"
#include "vector_3.h"
#include "vector_4.h"
#include "matrix_4x4.h"

namespace mathfu {


using Vector2f = mathfu::Vector<float, 2>;
using Vector3f = mathfu::Vector<float, 3>;
using Vector4f = mathfu::Vector<float, 4>;

using Vector2d = mathfu::Vector<double, 2>;
using Vector3d = mathfu::Vector<double, 3>;
using Vector4d = mathfu::Vector<double, 4>;

/// 4 rows by 4 columns float matrix.
using Matrix4x4f = mathfu::Matrix<float, 4, 4>;
/// 3 rows by 4 columns float matrix.
using Matrix3x4f = mathfu::Matrix<float, 3, 4>;
/// 4 rows by 3 columns float matrix.
using Matrix4x3f = mathfu::Matrix<float, 4, 3>;
/// 3 rows by 3 columns float matrix.
using Matrix3x3f = mathfu::Matrix<float, 3, 3>;


using Quaternionf = mathfu::Quaternion<float>;


} // namespace mathfu

