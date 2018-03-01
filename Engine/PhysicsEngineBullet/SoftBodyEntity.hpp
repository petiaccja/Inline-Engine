#pragma once
#include "PhysicsEngine/ISoftBodyEntity.hpp"
#include <Bullet/BulletSoftBody/btSoftBody.h>

class btSoftBody;

namespace inl::physics::bullet {

class SoftBodyEntity : public inl::physics::ISoftBodyEntity
{
public:
	SoftBodyEntity(btSoftBody* body);

	inline uint64_t GetCollisionGroup() const override  { return collisionGroupID; }

	inline btSoftBody* GetBody() const { return body; }

protected:
	btSoftBody* body;

	uint64_t collisionGroupID;
	void* userPointer;
};

} // Namespace end  physics::bullet