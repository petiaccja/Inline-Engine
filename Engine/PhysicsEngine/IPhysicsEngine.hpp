#pragma once
#include "Common.hpp"

#include <InlineMath.hpp>
#include <vector>
#include <stdint.h>

using namespace inl;
using namespace inl::physics;

namespace inl::physics {

class IPhysicsEngine
{
public:
	virtual void Release() = 0;

	virtual void Update(float deltaTime) = 0;

	virtual bool TraceClosestPoint(const Vec3& from, const Vec3& to, TraceResult& traceInfo_out, const TraceParams& params = TraceParams()) = 0;

	// Create, Add DYNAMIC rigid body to physics world
	virtual IRigidBodyEntity* AddEntityRigidDynamic(Vec3* vertices, uint32_t nVertices, float mass = 1) = 0;

	// Create, Add STATIC rigid body to physics world
	virtual IRigidBodyEntity* AddEntityRigidStatic(Vec3* vertices, uint32_t nVertices, void* indices, uint32_t indexSize, uint32_t nIndices) = 0;

	// Create, Add capsule rigid body to physics world
	virtual IRigidBodyEntity* AddEntityRigidCapsule(float height, float radius, float mass) = 0;

	// Remove rigidBody entity from world
	virtual bool RemoveEntity(IRigidBodyEntity* e) = 0;

	virtual void SetLayerCollision(size_t ID0, size_t ID1, bool bEnableCollision) = 0;

	virtual bool IsLayersCanCollide(size_t ID0, size_t ID1) const = 0;

	virtual std::vector<Collision>& GetCollisionList() = 0;

	virtual bool GetDebugData(Vec3*& linesFromNonUniqPoints_out, size_t& nLines_out) const = 0;
};

}
