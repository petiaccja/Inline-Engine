#pragma once

#include "PhysicsEngine/IRigidBodyEntity.hpp"

#include <Bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <Bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <Bullet/BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <Bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <Bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <Bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h>


#include <InlineMath.hpp>

namespace inl::physics::bullet {

class RigidBodyEntity : public inl::physics::IRigidBodyEntity
{
public:
	RigidBodyEntity(btRigidBody* body, btCollisionWorld* world);

	void ApplyForce(const Vec3& force, const Vec3& relPos = { 0, 0, 0 }) override;

	void SetUserPointer(void* p) override;

	void SetGravityScale(float s) override;
	void SetTrigger(bool bTrigger) override;
	void SetCollisionGroup(int64_t ID) override;

	void SetAngularFactor(float f) override;
	void SetKinematic(bool b) override;
	void SetVelocity(const Vec3& v) override;

	void SetPos(const Vec3& v) override;
	void SetRot(const Quat& q) override;
	void SetScale(const Vec3& v) override;
	void SetSkew(const Mat33& skew) override;

	const Vec3 GetPos() const override;
	const Quat GetRot() const override;
	const Vec3 GetScale() const override;

	uint64_t GetCollisionGroup() const override;
	Vec3 GetVelocity() const override;
	void* GetUserPointer() override;

	std::vector<Contact> GetContacts() const override;

	bool IsTrigger()		const override;
	bool IsStatic()		const override;
	bool IsDynamic()		const override;
	bool IsKinematic()	const override;

	btRigidBody* GetBody();

protected:
	btRigidBody* body;

	// Thank you Bullet physics, your design fucked up ours, warning ugly code !
	btCollisionWorld* world;

	int64_t collisionGroupID;
	void* userPointer;
};

} // namespace inl::physics::bullet