#pragma once
#include "WorldComponent.hpp"

#include "PhysicsEngine/IRigidBodyEntity.hpp"

class RigidBodyComponent : public WorldComponent
{
public:
	static const eWorldComponentType TYPE = RIGID_BODY;

public:
	RigidBodyComponent(physics::IRigidBodyEntity* a);

	void AddForce(const Vec3& force, const Vec3& relPos = { 0, 0, 0 });

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

	physics::IRigidBodyEntity* GetEntity();

	std::vector<physics::ContactPoint> GetContactPoints() const;

protected:
	physics::IRigidBodyEntity* entity;
};