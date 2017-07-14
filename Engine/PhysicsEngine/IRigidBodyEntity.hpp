#pragma once
#include "PhysicsEngine/Common.hpp"

#include <InlineMath.hpp>

#include <vector>
#include <stdint.h>

namespace inl::physics {

class IRigidBodyEntity
{
public:
	virtual void AddForce(const Vec3& force, const Vec3& relPos = {0,0,0}) = 0;

	virtual void SetUserPointer(void* ptr) = 0;
	virtual void SetGravityScale(float s) = 0;
	virtual void SetTrigger(bool bTrigger) = 0;
	virtual void SetCollisionGroup(int64_t ID) = 0;

	virtual void SetAngularFactor(float factor) = 0;
	virtual void SetKinematic(bool bKinematic) = 0;
	virtual void SetVelocity(const Vec3& v) = 0;

	virtual void SetPos(const Vec3& v) = 0;
	virtual void SetRot(const Quat& q) = 0;
	virtual void SetScale(const Vec3& v) = 0;
	virtual void SetSkew(const Mat33& skew) = 0;

	virtual const Vec3 GetPos() const = 0;
	virtual const Quat GetRot() const = 0;
	virtual const Vec3 GetScale() const = 0;

	virtual uint64_t GetCollisionGroup() const = 0;
	virtual Vec3 GetVelocity() const = 0;
	virtual void* GetUserPointer() = 0;

	virtual std::vector<Contact> GetContacts() const = 0;

	virtual bool IsTrigger() const = 0;
	virtual bool IsStatic() const = 0;
	virtual bool IsDynamic() const = 0;
	virtual bool IsKinematic() const = 0;
};

} // namespace inl::physics