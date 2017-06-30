#pragma once

#include "PhysicsEngine/IPhysicsEngine.hpp"
#include "PhysicsEngine/IRigidBodyEntity.hpp"
#include "RigidBodyEntity.hpp"
#include "SoftBodyEntity.hpp"

#include <Bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>
#include <Bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <Bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <Bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include <InlineMath.hpp>
#include <vector>

using namespace inl;
using namespace inl::physics::bullet;

class btSoftRigidDynamicsWorld;
class btDiscreteDynamicsWorld;

namespace inl::physics::bullet {

struct PhysicsEngineBulletDesc
{
	PhysicsEngineBulletDesc() : gravity(0, 0, 0) {}

	Vec3 gravity;
};

class PhysicsEngineBullet : public IPhysicsEngine
{
public:
	PhysicsEngineBullet(const PhysicsEngineBulletDesc& d);
	~PhysicsEngineBullet();
	void Release() override;

	void Update(float deltaTime) override;

	bool TraceClosestPoint(const Vec3& from, const Vec3& to, physics::TraceResult& traceResult_out, const physics::TraceParams& params = physics::TraceParams()) override;

	// Create, Add DYNAMIC rigid body to physics world
	physics::IRigidBodyEntity* AddEntityRigidDynamic(Vec3* vertices, uint32_t nVertices, float mass = 1) override;

	// Create, Add STATIC rigid body to physics world
	physics::IRigidBodyEntity* AddEntityRigidStatic(Vec3* vertices, uint32_t nVertices, void* indices, uint32_t indexStride, uint32_t nIndices) override;

	// Create, Add capsule rigid body to physics world
	physics::IRigidBodyEntity* AddEntityRigidCapsule(float height, float radius, float mass) override;

	bool RemoveEntity(physics::IRigidBodyEntity* e) override;

	void SetLayerCollision(size_t ID0, size_t ID1, bool bEnableCollision) override;

	bool IsLayersCanCollide(size_t ID0, size_t ID1) const override
	{
		assert(ID0 < sqrt(layeCollisionMatrix.size()));
		assert(ID1 < sqrt(layeCollisionMatrix.size()));

		return layeCollisionMatrix[ID0 + ID1 * nLayeCollisionMatrixRows] > 0;
	}

	std::vector<physics::Collision>& GetCollisionList() override { return contactList; }

	bool GetDebugData(Vec3*& linesFromNonUniqPoints_out, size_t& nLines_out) const override;

private:
	btDiscreteDynamicsWorld* world;

	std::vector<physics::IRigidBodyEntity*> entities;

	std::vector<physics::Collision> contactList;

	// byte bool array bitch pls
	std::vector<uint8_t> layeCollisionMatrix;
	size_t nLayeCollisionMatrixRows;
};


class BulletCollisionDispatcher : public btCollisionDispatcher
{
public:
	BulletCollisionDispatcher(btCollisionConfiguration* c, PhysicsEngineBullet* p)
		:btCollisionDispatcher(c), physicsEngine(p)
	{

	}

	bool needsCollision(const btCollisionObject* bodyA, const btCollisionObject* bodyB) override
	{
		bool bCollide = btCollisionDispatcher::needsCollision(bodyA, bodyB);

		uint64_t colGroupA;
		uint64_t colGroupB;

		if (bCollide)
		{
			if (!bodyA->getCollisionShape()->isSoftBody())
				colGroupA = ((RigidBodyEntity*)bodyA->getUserPointer())->GetCollisionGroup();
			else
				colGroupA = ((SoftBodyEntity*)bodyA->getUserPointer())->GetCollisionGroup();

			if (!bodyB->getCollisionShape()->isSoftBody())
				colGroupB = ((RigidBodyEntity*)bodyB->getUserPointer())->GetCollisionGroup();
			else
				colGroupB = ((SoftBodyEntity*)bodyB->getUserPointer())->GetCollisionGroup();
		}

		bCollide &= colGroupA == std::numeric_limits<uint64_t>::max() || colGroupB == std::numeric_limits<uint64_t>::max() || physicsEngine->IsLayersCanCollide(colGroupA, colGroupB);

		return bCollide;
	}

protected:
	PhysicsEngineBullet* physicsEngine;
};


struct ClosestRayCallback : public btCollisionWorld::ClosestRayResultCallback
{
	ClosestRayCallback(IPhysicsEngine* p, const btVector3& rayFromWorld, const btVector3& rayToWorld, const std::vector<size_t>& ignoredCollisionLayers)
	:btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld), ignoredCollisionLayers(ignoredCollisionLayers), physicsEngine(p)
	{
	}

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const override
	{
		btCollisionObject* col = (btCollisionObject*)proxy0->m_clientObject;

		size_t colGroup;
		if (!col->getCollisionShape()->isSoftBody())
			colGroup = ((RigidBodyEntity*)col->getUserPointer())->GetCollisionGroup();
		else
			colGroup = ((SoftBodyEntity*)col->getUserPointer())->GetCollisionGroup();

		size_t i = 0;
		for (; i < ignoredCollisionLayers.size(); i++)
			if (ignoredCollisionLayers[i] == colGroup)
				break;

		return i == ignoredCollisionLayers.size();
	}

protected:
	IPhysicsEngine* physicsEngine;

	std::vector<size_t> ignoredCollisionLayers;
};

}