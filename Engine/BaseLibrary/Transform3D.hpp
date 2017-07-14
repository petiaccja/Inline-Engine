#pragma once
#include <InlineMath.hpp>

namespace inl {

class Transform3D
{
public:
	Transform3D();
	Transform3D(const Vec3& pos, const Quat& rot, const Vec3& scale);
	Transform3D(const Transform3D& from, const Transform3D& to);

	void SetPos(const Vec3& v);
	void SetRot(const Quat& q);
	void SetRot(const Quat& q, const Vec3& rotOrigin);
	void SetScale(const Vec3& v);
	void SetScale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot);
	void SetSkew(const Mat33& m);
	
	void Move(const Vec3& v);
	void Rot(const Quat& q);
	void Rot(const Quat& q, const Vec3& rotOrigin);
	void Scale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot);
	void Scale(const Vec3& scale);
	
	const Vec3& GetPos() const;
	const Quat& GetRot() const;
	const Mat33& GetSkew() const;
	const Vec3 GetScale() const;
	
	Vec3 GetFrontDir()	const;
	Vec3 GetBackDir()	const;
	Vec3 GetUpDir()		const;
	Vec3 GetDownDir()	const;
	Vec3 GetRightDir()	const;
	Vec3 GetLeftDir()	const;
	
	Transform3D operator * (const Transform3D& b);

protected:
	Vec3 pos;
	Quat rot;
	Mat33 skew;
};

} // namespace inl