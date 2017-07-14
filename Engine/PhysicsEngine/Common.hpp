#pragma once

#include <vector>

#include <InlineMath.hpp>

namespace inl::physics {

class IRigidBodyEntity;
class ISoftBodyEntity;

struct Contact
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

	std::vector<Contact> contacts;
};

struct TraceResult
{
	void* userPointer;

	Vec3 pos;
	Vec3 normal;
};

struct TraceParams
{
	void AddIgnoreCollisionLayer(uint64_t collisionLayerID) 
	{ 
		ignoredCollisionLayers.push_back(collisionLayerID);
	}
	
	std::vector<uint64_t> ignoredCollisionLayers;
};

} // namespace inl::physics