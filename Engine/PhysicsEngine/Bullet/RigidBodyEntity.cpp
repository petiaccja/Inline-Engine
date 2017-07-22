#include "RigidBodyEntity.hpp"

using namespace inl;
using namespace inl::physics;
using namespace inl::physics::bullet;

RigidBodyEntity::RigidBodyEntity(btRigidBody* body, btCollisionWorld* world)
:body(body), world(world), userPointer(0), collisionGroupID(std::numeric_limits<uint64_t>::max()) // default ID means can collide with everything
{
	body->setUserPointer(this);
	//btConvexHullShape asd;
	//asd.getVertex
	//body->getCollisionShape()
	//btBvhTriangleMeshShape asd;
	//asd.getMeshInterface()->getLockedVertexIndexBase
}

void RigidBodyEntity::ApplyForce(const Vec3& force, const Vec3& relPos /*= {0,0,0}*/)
{
	body->applyForce({ force.x, force.y, force.z }, { relPos.x, relPos.y, relPos.z });
	body->activate();
}

void RigidBodyEntity::SetUserPointer(void* p)
{
	userPointer = p;
}

void RigidBodyEntity::SetGravityScale(float s)
{
	body->setGravity(body->getGravity() * s);
}

void RigidBodyEntity::SetTrigger(bool bTrigger)
{
	if (bTrigger)
		body->setCollisionFlags(body->getCollisionFlags() | btRigidBody::CF_NO_CONTACT_RESPONSE);
	else
		body->setCollisionFlags(body->getCollisionFlags() & ~btRigidBody::CF_NO_CONTACT_RESPONSE);
}

void RigidBodyEntity::SetCollisionGroup(int64_t ID)
{
	collisionGroupID = ID;
}

void RigidBodyEntity::SetAngularFactor(float f)
{
	body->setAngularFactor(f);
}

void RigidBodyEntity::SetKinematic(bool b)
{
	if (b)
		body->setFlags(body->getFlags() | btRigidBody::CF_KINEMATIC_OBJECT);
	else
		body->setFlags(body->getFlags() & ~btRigidBody::CF_KINEMATIC_OBJECT);
}

void RigidBodyEntity::SetVelocity(const Vec3& v)
{
	if (body->getInvMass() != 0)
		body->setLinearVelocity({ v.x, v.y, v.z });
}

void RigidBodyEntity::SetPos(const Vec3& v)
{
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	trans.setOrigin(btVector3(v.x, v.y, v.z));
	body->setWorldTransform(trans);
	body->getMotionState()->setWorldTransform(trans);
	body->activate();
}

void RigidBodyEntity::SetRot(const Quat& q)
{
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	btQuaternion btQuat;
	btQuat.setX(q.x);
	btQuat.setY(q.y);
	btQuat.setZ(q.z);
	btQuat.setW(q.w);
	trans.setRotation(btQuat);

	body->setWorldTransform(trans);
	body->getMotionState()->setWorldTransform(trans);
	body->activate();
}

void RigidBodyEntity::SetScale(const Vec3& v)
{
	btCollisionShape* colShape = body->getCollisionShape();
	assert(colShape);

	if (v.x == 1)
		return;

	colShape->setLocalScaling(btVector3(v.x, v.y, v.z));

	// I think it's needed
	btVector3 localInertia(0, 0, 0);
	float invMass = body->getInvMass();
	if (invMass != 0)
	{
		float mass = 1.f / invMass;
		colShape->calculateLocalInertia(mass, localInertia);
		body->setMassProps(mass, localInertia);
	}

	world->updateSingleAabb(body);
	body->activate();
}

void RigidBodyEntity::SetSkew(const Mat33& skew)
{
	btCollisionShape* baseShape = body->getCollisionShape();
	int shapeType = baseShape->getShapeType();

	switch (shapeType)
	{
	case CONVEX_HULL_SHAPE_PROXYTYPE:
	{
		btConvexHullShape* shape = (btConvexHullShape*)body->getCollisionShape();

		btVector3* vertices = shape->getUnscaledPoints();
		for (int i = 0; i < shape->getNumPoints(); i++)
		{
			Vec3 transformedVertex = skew * Vec3(vertices[i].x(), vertices[i].y(), vertices[i].z());
			vertices[i] = btVector3(transformedVertex.x, transformedVertex.y, transformedVertex.z);

			shape->recalcLocalAabb();
		}
	}break;
	case CAPSULE_SHAPE_PROXYTYPE:
	{
		btCapsuleShape* shape = (btCapsuleShape*)body->getCollisionShape();

		Vec3 scale = GetScale();

		shape->setLocalScaling({ scale.x, scale.y, scale.z });
	}break;

	case BOX_SHAPE_PROXYTYPE:
	case TRIANGLE_SHAPE_PROXYTYPE:
	case TETRAHEDRAL_SHAPE_PROXYTYPE:
	case CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE:
	case CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE:
	case CUSTOM_POLYHEDRAL_SHAPE_TYPE:
	case IMPLICIT_CONVEX_SHAPES_START_HERE:
	case SPHERE_SHAPE_PROXYTYPE:
	case MULTI_SPHERE_SHAPE_PROXYTYPE:
	case CONE_SHAPE_PROXYTYPE:
	case CONVEX_SHAPE_PROXYTYPE:
	case CYLINDER_SHAPE_PROXYTYPE:
	case UNIFORM_SCALING_SHAPE_PROXYTYPE:
	case MINKOWSKI_SUM_SHAPE_PROXYTYPE:
	case MINKOWSKI_DIFFERENCE_SHAPE_PROXYTYPE:
	case BOX_2D_SHAPE_PROXYTYPE:
	case CONVEX_2D_SHAPE_PROXYTYPE:
	case CUSTOM_CONVEX_SHAPE_TYPE:
	case CONCAVE_SHAPES_START_HERE:
	case TRIANGLE_MESH_SHAPE_PROXYTYPE:
	case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE:
	case FAST_CONCAVE_MESH_PROXYTYPE:
	case TERRAIN_SHAPE_PROXYTYPE:
	case GIMPACT_SHAPE_PROXYTYPE:
	case MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE:
	case EMPTY_SHAPE_PROXYTYPE:
	case STATIC_PLANE_PROXYTYPE:
	case CUSTOM_CONCAVE_SHAPE_TYPE:
	case CONCAVE_SHAPES_END_HERE:
	case COMPOUND_SHAPE_PROXYTYPE:
	case SOFTBODY_SHAPE_PROXYTYPE:
	case HFFLUID_SHAPE_PROXYTYPE:
	case HFFLUID_BUOYANT_CONVEX_SHAPE_PROXYTYPE:
	case INVALID_SHAPE_PROXYTYPE:
	case MAX_BROADPHASE_COLLISION_TYPES:
		break;
	}
}

const Vec3 RigidBodyEntity::GetPos() const
{
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	return Vec3(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());
}

const Quat RigidBodyEntity::GetRot() const
{
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);

	Quat rot;
	rot.x = trans.getRotation().x();
	rot.y = trans.getRotation().y();
	rot.z = trans.getRotation().z();
	rot.w = trans.getRotation().w();
	return rot;
}

const Vec3 RigidBodyEntity::GetScale() const
{
	btCollisionShape* shape = body->getCollisionShape();

	if (shape)
		return Vec3(shape->getLocalScaling().x(), shape->getLocalScaling().y(), shape->getLocalScaling().z());

	return Vec3(1, 1, 1);
}

uint64_t RigidBodyEntity::GetCollisionGroup() const
{
	return collisionGroupID;
}

Vec3 RigidBodyEntity::GetVelocity() const
{
	return Vec3(body->getLinearVelocity().x(), body->getLinearVelocity().y(), body->getLinearVelocity().z());
}

void* RigidBodyEntity::GetUserPointer()
{
	return userPointer;
}

std::vector<Contact> RigidBodyEntity::GetContacts() const
{
	std::vector<Contact> tmp;
	return tmp;
}

bool RigidBodyEntity::IsTrigger() const
{
	return (body->getCollisionFlags() & btRigidBody::CF_NO_CONTACT_RESPONSE) != 0;
}

bool RigidBodyEntity::IsStatic() const
{
	return body->isStaticObject();
}

bool RigidBodyEntity::IsDynamic() const
{
	return !body->isStaticObject();
}

bool RigidBodyEntity::IsKinematic() const
{
	return body->isKinematicObject();
}

btRigidBody* RigidBodyEntity::GetBody()
{
	return body;
}
