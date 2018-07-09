#pragma once

// Utility files for converting between bullet math types and proper math types.

#include <InlineMath.hpp>
#include <Bullet/LinearMath/btQuaternion.h>
#include <Bullet/LinearMath/btVector3.h>
#include <Bullet/LinearMath/btMatrix3x3.h>


namespace inl::pxeng_bl {


inline Vec3 Conv(const btVector3& v) {
	return { v.x(), v.y(), v.z() };
}

inline btVector3 Conv(const Vec3& v) {
	return { v.x, v.y, v.z };
}

inline Quat Conv(const btQuaternion& q) {
	return { q.getW(), q.getX(), q.getY(), q.getZ() };
}

inline btQuaternion Conv(const Quat& q) {
	return { q.x, q.y, q.z, q.w };
}


inline Mat33 Conv(const btMatrix3x3& m) {
	Mat33 ret;
	for (int j=0; j<3; ++j) {
		for (int i = 0; i<3; ++i) {
			ret(i, j) = m[i][j];
		}
	}
	return ret;
}

inline btMatrix3x3 Conv(const Mat33& m) {
	btMatrix3x3 ret;
	for (int j = 0; j<3; ++j) {
		for (int i = 0; i<3; ++i) {
			ret[i][j] = m(i, j);
		}
	}
	return ret;
}


} // namespace inl::pxeng_bl