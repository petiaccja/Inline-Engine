#pragma once

#include "Mathter/Vector.hpp"
#include "Mathter/Matrix.hpp"
#include "Mathter/Quaternion.hpp"
#include "Mathter/Utility.hpp"
#include "Mathter/Geometry.hpp"
#include <cstdint>


namespace inl {


// Introduce mathter into inl namespace.
using namespace mathter;


// Global matrix properties
namespace matrix_props {

static constexpr eMatrixLayout layout = eMatrixLayout::ROW_MAJOR;
static constexpr eMatrixOrder order = eMatrixOrder::FOLLOW_VECTOR;

}


// Common definitions for vectors.
using Vec2 = Vector<float, 2, false>;
using Vec3 = Vector<float, 3, false>;
using Vec4 = Vector<float, 4, false>;

using Vec2d = Vector<double, 2, false>;
using Vec3d = Vector<double, 3, false>;
using Vec4d = Vector<double, 4, false>;

using Vec2i = Vector<int, 2, false>;
using Vec3i = Vector<int, 3, false>;
using Vec4i = Vector<int, 4, false>;

using Vec2u = Vector<unsigned, 2, false>;
using Vec3u = Vector<unsigned, 3, false>;
using Vec4u = Vector<unsigned, 4, false>;

using Vec2i64 = Vector<int64_t, 2, false>;
using Vec3i64 = Vector<int64_t, 3, false>;
using Vec4i64 = Vector<int64_t, 4, false>;

using Vec2u64 = Vector<uint64_t, 2, false>;
using Vec3u64 = Vector<uint64_t, 3, false>;
using Vec4u64 = Vector<uint64_t, 4, false>;


// Common definitions for packed vectors.
using Vec2_Packed = Vector<float, 2, true>;
using Vec3_Packed = Vector<float, 3, true>;
using Vec4_Packed = Vector<float, 4, true>;

using Vec2d_Packed = Vector<double, 2, true>;
using Vec3d_Packed = Vector<double, 3, true>;
using Vec4d_Packed = Vector<double, 4, true>;

using Vec2i_Packed = Vector<int, 2, true>;
using Vec3i_Packed = Vector<int, 3, true>;
using Vec4i_Packed = Vector<int, 4, true>;

using Vec2u_Packed = Vector<unsigned, 2, true>;
using Vec3u_Packed = Vector<unsigned, 3, true>;
using Vec4u_Packed = Vector<unsigned, 4, true>;

using Vec2i64_Packed = Vector<int64_t, 2, true>;
using Vec3i64_Packed = Vector<int64_t, 3, true>;
using Vec4i64_Packed = Vector<int64_t, 4, true>;

using Vec2u64_Packed = Vector<uint64_t, 2, true>;
using Vec3u64_Packed = Vector<uint64_t, 3, true>;
using Vec4u64_Packed = Vector<uint64_t, 4, true>;


// Common definitions for matrices.
using Mat22 = Matrix<float, 2, 2, matrix_props::order, matrix_props::layout, false>;
using Mat33 = Matrix<float, 3, 3, matrix_props::order, matrix_props::layout, false>;
using Mat44 = Matrix<float, 4, 4, matrix_props::order, matrix_props::layout, false>;
using Mat34 = Matrix<float, 3, 4, matrix_props::order, matrix_props::layout, false>;
using Mat43 = Matrix<float, 4, 3, matrix_props::order, matrix_props::layout, false>;

using Mat22d = Matrix<double, 2, 2, matrix_props::order, matrix_props::layout, false>;
using Mat33d = Matrix<double, 3, 3, matrix_props::order, matrix_props::layout, false>;
using Mat44d = Matrix<double, 4, 4, matrix_props::order, matrix_props::layout, false>;
using Mat34d = Matrix<double, 3, 4, matrix_props::order, matrix_props::layout, false>;
using Mat43d = Matrix<double, 4, 3, matrix_props::order, matrix_props::layout, false>;


// Common definitions for packed matrices.
using Mat22_Packed = Matrix<float, 2, 2, matrix_props::order, matrix_props::layout, true>;
using Mat33_Packed = Matrix<float, 3, 3, matrix_props::order, matrix_props::layout, true>;
using Mat44_Packed = Matrix<float, 4, 4, matrix_props::order, matrix_props::layout, true>;
using Mat34_Packed = Matrix<float, 3, 4, matrix_props::order, matrix_props::layout, true>;
using Mat43_Packed = Matrix<float, 4, 3, matrix_props::order, matrix_props::layout, true>;

using Mat22d_Packed = Matrix<double, 2, 2, matrix_props::order, matrix_props::layout, true>;
using Mat33d_Packed = Matrix<double, 3, 3, matrix_props::order, matrix_props::layout, true>;
using Mat44d_Packed = Matrix<double, 4, 4, matrix_props::order, matrix_props::layout, true>;
using Mat34d_Packed = Matrix<double, 3, 4, matrix_props::order, matrix_props::layout, true>;
using Mat43d_Packed = Matrix<double, 4, 3, matrix_props::order, matrix_props::layout, true>;

// Common definitions for quaternions
using Quat = mathter::Quaternion<float, false>;
using Quatd = mathter::Quaternion<double, false>;
using Quat_Packed = mathter::Quaternion<float, true>;
using Quatd_Packed = mathter::Quaternion<double, true>;


// Misc stuff
using Plane = mathter::Hyperplane<float, 3>;
using Planed = mathter::Hyperplane<double, 3>;

using Ray2D = mathter::Ray<float, 2>;
using Ray2Dd = mathter::Ray<double, 2>;
using Ray3D = mathter::Ray<float, 3>;
using Ray3Dd = mathter::Ray<double, 3>;

using Line2D = mathter::Line<float, 2>;
using Line2Dd = mathter::Line<double, 2>;
using Line3D = mathter::Line<float, 3>;
using Line3Dd = mathter::Line<double, 3>;

using LineSegment2D = mathter::LineSegment<float, 2>;
using LineSegment2Dd = mathter::LineSegment<double, 2>;
using LineSegment3D = mathter::LineSegment<float, 3>;
using LineSegment3Dd = mathter::LineSegment<double, 3>;




} // namespace inl