#include "SoftBodyEntity.hpp"

#include <Bullet/BulletSoftBody/btSoftBody.h>

using namespace inl::physics::bullet;

SoftBodyEntity::SoftBodyEntity(btSoftBody* body)
:body(body), collisionGroupID(-1), userPointer(0) // -1 default means can collide with everything
{
	body->setUserPointer(this);
}