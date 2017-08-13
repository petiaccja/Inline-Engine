#pragma once
#include "Part.hpp"
#include <PhysicsEngine/IRigidBodyEntity.hpp>

namespace inl::core {

using namespace inl;
using physics::Contact;

class RigidBodyPart : virtual public Part
{
public:
	static const ePartType TYPE = RIGID_BODY;

public:
	RigidBodyPart(Scene* scene, physics::IRigidBodyEntity* a);

	void ApplyForce(const Vec3& force, const Vec3& relPos = { 0, 0, 0 });

	void SetUserPointer(void* p);

	void SetGravityScale(float s);
	void SetTrigger(bool bTrigger);
	void SetCollisionGroup(uint64_t ID);

	void SetAngularFactor(float factor);
	void SetKinematic(bool bKinematic);
	void SetVelocity(const Vec3& v);

	uint64_t GetCollisionGroup() const;
	Vec3 GetVelocity() const;
	void* GetUserPointer();

	bool IsTrigger() const;
	bool IsStatic() const;
	bool IsDynamic() const;
	bool IsKinematic() const;

	std::vector<Contact> GetContacts() const;

protected:
	physics::IRigidBodyEntity* entity;
};

}