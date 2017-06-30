#pragma once

#include <vector>

#include <InlineMath.hpp>

namespace inl::physics {

class IRigidBodyEntity;
class ISoftBodyEntity;

struct ContactPoint
{
	Vec3 normalA;
	Vec3 normalB;
	Vec3 posA;
	Vec3 posB;
};

struct Collision
{
	IRigidBodyEntity* rigidBodyA;
	IRigidBodyEntity* rigidBodyB;

	ISoftBodyEntity* softBodyA;
	ISoftBodyEntity* softBodyB;

	std::vector<ContactPoint> contacts;
};

struct TraceResult
{
	void* userPointer;

	Vec3 pos;
	Vec3 normal;
};

struct TraceParams
{
	void AddIgnoreCollisionLayer(size_t collisionLayerID) 
	{ 
		ignoredCollisionLayers.push_back(collisionLayerID);
	}
	
	std::vector<size_t> ignoredCollisionLayers;
};

} // namespace inl::physics