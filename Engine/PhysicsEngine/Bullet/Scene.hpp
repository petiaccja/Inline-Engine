#pragma once

#include "PhysicsEngine/IRigidBodyEntity.hpp"
#include "RigidBodyEntity.hpp"
#include "SoftBodyEntity.hpp"

#include <Bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>
#include <Bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <Bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <Bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include <InlineMath.hpp>
#include <vector>


class btDiscreteDynamicsWorld;

namespace inl::physics::bullet {

class Scene
{
public:
	Scene(const Vec3& gravity = {0, 0, -9.81});
	~Scene();

	void Update(float deltaTime);

	bool TraceRay(const Ray3D& ray, physics::TraceResult& traceResult_out, float maxDistance = std::numeric_limits<float>::max(), const physics::TraceParams& params = physics::TraceParams());

	// Create, Add DYNAMIC rigid body to physics world
	physics::IRigidBodyEntity* AddEntityRigidDynamic(Vec3* vertices, uint32_t nVertices, float mass = 1);

	// Create, Add STATIC rigid body to physics world
	physics::IRigidBodyEntity* AddEntityRigidStatic(Vec3* vertices, uint32_t nVertices, void* indices, uint32_t indexStride, uint32_t nIndices);

	// Create, Add capsule rigid body to physics world
	physics::IRigidBodyEntity* AddEntityRigidCapsule(float height, float radius, float mass);

	bool RemoveEntity(physics::IRigidBodyEntity* e);

	void SetLayerCollision(uint64_t ID0, uint64_t ID1, bool bEnableCollision);

	bool IsLayersCanCollide(size_t ID0, size_t ID1) const
	{
		assert(ID0 < sqrt(layeCollisionMatrix.size()));
		assert(ID1 < sqrt(layeCollisionMatrix.size()));

		return layeCollisionMatrix[ID0 + ID1 * nLayeCollisionMatrixRows] > 0;
	}

	std::vector<physics::Collision>& GetCollisionList() { return contactList; }

	bool GetDebugData(Vec3*& linesFromNonUniqPoints_out, size_t& nLines_out) const;

protected:
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
	BulletCollisionDispatcher(btCollisionConfiguration* c, Scene* p)
		:btCollisionDispatcher(c), scene(p)
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

		bCollide &= colGroupA == std::numeric_limits<uint64_t>::max() || colGroupB == std::numeric_limits<uint64_t>::max() || scene->IsLayersCanCollide(colGroupA, colGroupB);

		return bCollide;
	}

protected:
	Scene* scene;
};


struct ClosestRayCallback : public btCollisionWorld::ClosestRayResultCallback
{
	ClosestRayCallback(const btVector3& rayStart, const btVector3& rayEnd, const std::vector<size_t>& ignoredCollisionLayers)
	:btCollisionWorld::ClosestRayResultCallback(rayStart, rayEnd), ignoredCollisionLayers(ignoredCollisionLayers)
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
	std::vector<size_t> ignoredCollisionLayers;
};

} // namespace inl::core::physics
