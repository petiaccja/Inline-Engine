#pragma once
#include "InlineMath.hpp"

using namespace inl;

class Transform3D
{
public:
	inline Transform3D();
	inline Transform3D(const Vec3& pos, const Quat& rot, const Vec3& scale);
	inline Transform3D(const Transform3D& from, const Transform3D& to);

	inline void SetPos(const Vec3& v);
	inline void SetRot(const Quat& q);
	inline void SetRot(const Quat& q, const Vec3& rotOrigin);
	inline void SetScale(const Vec3& v);
	inline void SetScale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot);
	inline void SetSkew(const Mat33& m);
	
	inline  void Move(const Vec3& v);
	inline  void Rot(const Quat& q);
	inline  void Rot(const Quat& q, const Vec3& rotOrigin);
	inline  void Scale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot);
	inline  void Scale(const Vec3& scale);

	inline  const Vec3& GetPos() const;
	inline  const Quat& GetRot() const;
	inline  const Mat33& GetSkew() const;
	inline  const Vec3  GetScale() const;

	inline  Vec3 GetFrontDir()	const;
	inline  Vec3 GetBackDir()	const;
	inline  Vec3 GetUpDir()		const;
	inline  Vec3 GetDownDir()	const;
	inline  Vec3 GetRightDir()	const;
	inline  Vec3 GetLeftDir()	const;

	inline  Transform3D operator * (const Transform3D& b);

protected:
	Vec3 pos;
	Quat rot;
	Mat33 skew;
};

Transform3D::Transform3D()
:pos(0, 0, 0)
{
	rot = Quat::AxisAngle(Vec3( 0, 1, 0 ), 0);
	skew = Mat33::Identity();
}

Transform3D::Transform3D(const Vec3& pos, const Quat& rot, const Vec3& scale)
:pos(pos), rot(rot)
{
	skew = Mat33::Scale(scale);
}

Transform3D::Transform3D(const Transform3D& from, const Transform3D& to)
{
	SetRot(to.GetRot() * from.GetRot().Inverse());
	SetPos(from.GetRot().Inverse() * (to.GetPos() - from.GetPos()) / from.GetScale());
	SetSkew(to.GetSkew() * from.GetSkew().Inverse());
}

void Transform3D::SetPos(const Vec3& v)
{
	pos = v;
}

void Transform3D::SetRot(const Quat& q)
{
	rot = q;
}

void Transform3D::SetRot(const Quat& q, const Vec3& rotOrigin)
{
	pos = rotOrigin + (rot.Inverse() * q) * (pos - rotOrigin);
	rot = q;
}

void Transform3D::Rot(const Quat& q, const Vec3& rotOrigin)
{
	pos = rotOrigin + q * (pos - rotOrigin);
	rot = q * rot;
}

void Transform3D::SetScale(const Vec3& v)
{
	skew *= Mat33::Scale(v);
}

void Transform3D::SetScale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot)
{
	Vec3 dScale = scale / GetScale();
	Scale(dScale, rootPos, rootRot);
}

void Transform3D::Scale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot)
{
	assert(0); // TODO Mather (Peti) Quat -> Mat33 conversion
	//Mat33 dRot = Mat33(rot * rootRot.Inverse());
	Mat33 dRot;

	pos = rootPos + rootRot * (rootRot.Inverse() * (pos - rootPos) * scale);
	skew = skew * dRot.Inverse() * Mat33::Scale(scale) * dRot;
}

void Transform3D::Scale(const Vec3& scale)
{
	skew = skew * Mat33::Scale(scale);
}

void Transform3D::SetSkew(const Mat33& m)
{
	skew = m;
}

void Transform3D::Move(const Vec3& v)
{
	SetPos(pos + v);
}

void Transform3D::Rot(const Quat& q)
{
	SetRot(rot * q);
}

const Vec3& Transform3D::GetPos() const
{
	return pos;
}

const Quat& Transform3D::GetRot() const
{
	return rot;
}

const Mat33& Transform3D::GetSkew() const
{
	return skew;
}

const Vec3 Transform3D::GetScale() const
{
	return skew * Vec3(1, 1, 1);
}

Vec3 Transform3D::GetFrontDir() const
{
	return (GetRot() * (Vec3(0, 1, 0) * GetSkew())).Normalized();
}

Vec3 Transform3D::GetBackDir() const
{
	return (GetRot() * (Vec3(0, -1, 0) * GetSkew())).Normalized();
}

Vec3 Transform3D::GetUpDir() const
{
	return (GetRot() * (Vec3(0, 0, 1) * GetSkew())).Normalized();
}

Vec3 Transform3D::GetDownDir() const
{
	return (GetRot() * (Vec3(0, 0, -1) * GetSkew())).Normalized();
}

Vec3 Transform3D::GetRightDir() const
{
	return (GetRot() * (Vec3(1, 0, 0) * GetSkew())).Normalized();
}

Vec3 Transform3D::GetLeftDir() const
{
	return (GetRot() * (Vec3(-1, 0, 0) * GetSkew())).Normalized();
}

Transform3D Transform3D::operator * (const Transform3D& b)
{
	// TODO
	assert(0);
	Transform3D result;
	return result;
}
