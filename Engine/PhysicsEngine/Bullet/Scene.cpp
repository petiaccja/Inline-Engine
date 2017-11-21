#include "Scene.hpp"

#include "RigidBodyEntity.hpp"
#include "SoftBodyEntity.hpp"
#include "BulletDebugDraw.hpp"

#include <Bullet/btBulletDynamicsCommon.h>
#include <Bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <Bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet/BulletSoftBody/btSoftBodyHelpers.h>
#include <Bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

namespace inl::physics::bullet {

Scene::Scene(const Vec3& gravity /*= { 0, 0, -9.81 }*/)
{
	world = new btDiscreteDynamicsWorld(new	BulletCollisionDispatcher(new btDefaultCollisionConfiguration, this),
		new btDbvtBroadphase,
		new btSequentialImpulseConstraintSolver,
		new btDefaultCollisionConfiguration);

	//world->getSolverInfo().m_numIterations = 4;
	//world->getSolverInfo().m_solverMode = SOLVER_SIMD;
	//world->getDispatchInfo().m_enableSPU = true;

	world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));

	btIDebugDraw* debugDrawer = (btIDebugDraw*)new BulletDebugDraw();
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb);

	//world->setDebugDrawer(debugDrawer);

	// Populate collisionMatrix with true values, everything can collide with everything by default
	nLayeCollisionMatrixRows = 16; // Start with 16x16 matrix
	layeCollisionMatrix.resize(nLayeCollisionMatrixRows * nLayeCollisionMatrixRows);
	memset(layeCollisionMatrix.data(), 1, nLayeCollisionMatrixRows * nLayeCollisionMatrixRows);
}

Scene::~Scene()
{
	/* Clean up	*/
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		while (world->getNumConstraints())
		{
			btTypedConstraint*	pc = world->getConstraint(0);
			world->removeConstraint(pc);
			delete pc;
		}
		btSoftBody* softBody = btSoftBody::upcast(obj);
		if (softBody)
		{
			//world->removeSoftBody(softBody);
		}
		else
		{
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body)
				world->removeRigidBody(body);
			else
				world->removeCollisionObject(obj);
		}
		world->removeCollisionObject(obj);
		delete obj;
	}

	delete world;
}

void Scene::Update(float deltaTime)
{
	{
		//PROFILE_SCOPE("Simulate");
		world->stepSimulation(deltaTime);
	}

	// TODO rework this global query into local Entity query
	//{
	//	//PROFILE_SCOPE("Contact List Query");
	//	contactList.clear();
	//
	//	int numManifolds = world->getDispatcher()->getNumManifolds();
	//	for (int i = 0; i < numManifolds; i++)
	//	{
	//		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
	//
	//		int numContacts = contactManifold->getNumContacts();
	//		if (numContacts != 0)
	//		{
	//			const btCollisionObject* colA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
	//			const btCollisionObject* colB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
	//
	//			// Fill up our structure with contact informations
	//			physics::Collision colInfo;
	//
	//			if (!colA->getCollisionShape()->isSoftBody())
	//			{
	//				colInfo.rigidBodyA = (RigidBodyEntity*)colA->getUserPointer();
	//				colInfo.softBodyA = nullptr;
	//			}
	//			else
	//			{
	//				colInfo.softBodyA = (SoftBodyEntity*)colA->getUserPointer();
	//				colInfo.rigidBodyA = nullptr;
	//			}
	//
	//
	//			if (!colB->getCollisionShape()->isSoftBody())
	//			{
	//				colInfo.rigidBodyB = (RigidBodyEntity*)colB->getUserPointer();
	//				colInfo.softBodyB = nullptr;
	//			}
	//			else
	//			{
	//				colInfo.softBodyB = (SoftBodyEntity*)colB->getUserPointer();
	//				colInfo.rigidBodyB = nullptr;
	//			}
	//
	//			for (int j = 0; j < numContacts; j++)
	//			{
	//				btManifoldPoint& pt = contactManifold->getContactPoint(j);
	//				if (pt.getDistance() <= 0.f)
	//				{
	//					//Fill contact data
	//					Contact c;
	//					c.normalA = -Vec3(pt.m_normalWorldOnB.x(), pt.m_normalWorldOnB.y(), pt.m_normalWorldOnB.z());
	//					c.normalB = Vec3(pt.m_normalWorldOnB.x(), pt.m_normalWorldOnB.y(), pt.m_normalWorldOnB.z());
	//					c.posA = Vec3(pt.m_positionWorldOnA.x(), pt.m_positionWorldOnA.y(), pt.m_positionWorldOnA.z());
	//					c.posB = Vec3(pt.m_positionWorldOnB.x(), pt.m_positionWorldOnB.y(), pt.m_positionWorldOnB.z());
	//					colInfo.contacts.push_back(c);
	//
	//					const btVector3& ptA = pt.getPositionWorldOnA();
	//					const btVector3& ptB = pt.getPositionWorldOnB();
	//					const btVector3& normalOnB = pt.m_normalWorldOnB;
	//				}
	//			}
	//
	//			if (colInfo.contacts.size() != 0)
	//				contactList.push_back(colInfo);
	//		}
	//	}
	//}

	if (world->getDebugDrawer())
	{
		((BulletDebugDraw*)world->getDebugDrawer())->ClearFrameData();
		world->debugDrawWorld();
	}
}

bool Scene::TraceRay(const Ray3D& ray, physics::TraceResult& traceResult_out, float maxDistance /*= std::numeric_limits<float>::max()*/, const physics::TraceParams& params /*= physics::TraceParams()*/)
{
	btVector3 rayStart = btVector3(ray.base.x, ray.base.y, ray.base.z);
	btVector3 rayEnd = rayStart + btVector3(ray.direction.x, ray.direction.y, ray.direction.z) * maxDistance;
	ClosestRayCallback function(rayStart, rayEnd, params.ignoredCollisionLayers);

	world->rayTest(rayStart, rayEnd, function);

	if (function.hasHit())
	{
		traceResult_out.normal = Vec3(function.m_hitNormalWorld.x(), function.m_hitNormalWorld.y(), function.m_hitNormalWorld.z());
		traceResult_out.pos = Vec3(function.m_hitPointWorld.x(), function.m_hitPointWorld.y(), function.m_hitPointWorld.z());
		traceResult_out.userPointer = function.m_collisionObject->getUserPointer();
		return true;
	}
	else
	{
		return false;
	}
}

physics::IRigidBodyEntity* Scene::AddEntityRigidDynamic(Vec3* vertices, uint32_t nVertices, float mass /*= 1*/)
{
	// You should call PhysicsEngineBullet::CreateEntityRigidStatic
	assert(mass != 0);

	// Create collision shape for rigid body, based on it's vertices and mass
	btConvexHullShape* colShape = new btConvexHullShape((btScalar*)vertices, nVertices, sizeof(Vec3));
	colShape->setSafeMargin(0.01, 1); // Thanks convex hull for your imprecision...

	btVector3 localInertia(0, 0, 0);
	if (mass != 0)
		colShape->calculateLocalInertia(mass, localInertia);

	// Create rigid body
	btRigidBody* body = new btRigidBody(mass, new btDefaultMotionState(), colShape, localInertia);

	if (mass > 0)
	{
		body->setCcdMotionThreshold(0.001);
		body->setCcdSweptSphereRadius(0.001);
	}
	body->setFriction(0.9f);
	body->setRestitution(0.0f);

	world->addRigidBody(body);

	RigidBodyEntity* e = new RigidBodyEntity(body, world);
	entities.push_back(e);

	return e;
}

physics::IRigidBodyEntity* Scene::AddEntityRigidStatic(Vec3* vertices, uint32_t nVertices, void* indices, uint32_t indexSize, uint32_t nIndices)
{
	btTriangleIndexVertexArray* VBIB;

	// Indices need copy, signed unsigne differences -.-
	// Okay bullet vector container equal size, let bullet read out pointer
	if (sizeof(Vec3) == sizeof(btVector3))
	{
		VBIB = new btTriangleIndexVertexArray(nIndices / 3, (int*)indices, 3 * indexSize, nVertices, (btScalar*)vertices, sizeof(btVector3));
	}
	else // Bullshit if Vec3 and btVector3 size not equal, we need into new array, Vec3.xyz to btVector3
	{

		int* myIndices = new int[nIndices];
		for (uint32_t i = 0; i < nIndices; i++)
			myIndices[i] = (int)*((uint32_t*)(indices)+i);

		btVector3* vertices_memCorrected = new btVector3[nVertices];
		for (uint32_t i = 0; i < nVertices; i++)
			memcpy(vertices_memCorrected[i], (unsigned char*)vertices + i * sizeof(Vec3), sizeof(Vec3));

		VBIB = new btTriangleIndexVertexArray(nIndices / 3, (int*)myIndices, 3 * indexSize, nVertices, (btScalar*)vertices_memCorrected, sizeof(btVector3));
	}

	// Create rigid body
	btRigidBody* body = new btRigidBody(0, new btDefaultMotionState(), new btBvhTriangleMeshShape(VBIB, true), btVector3(0, 0, 0));
	world->addRigidBody(body);

	RigidBodyEntity* e = new RigidBodyEntity(body, world);
	entities.push_back(e);

	return e;
}

physics::IRigidBodyEntity* Scene::AddEntityRigidCapsule(float height, float radius, float mass)
{
	btCapsuleShapeZ* capsuleShape = new btCapsuleShapeZ(radius, height);
	capsuleShape->setSafeMargin(0.01, 1); // Thanks convex hull for your imprecision...
										  // 0.04 a default
										  //btScalar ad = capsuleShape->getMargin();

	btVector3 localInertia(0, 0, 0);
	if (mass != 0)
		capsuleShape->calculateLocalInertia(mass, localInertia);

	// Create rigid body
	btRigidBody* body = new btRigidBody(mass, new btDefaultMotionState(), capsuleShape, localInertia);
	world->addRigidBody(body);

	RigidBodyEntity* e = new RigidBodyEntity(body, world);
	entities.push_back(e);

	return e;
}

bool Scene::RemoveEntity(physics::IRigidBodyEntity* e)
{
	auto it = std::find(entities.begin(), entities.end(), e);
	if (it != entities.end())
	{
		auto entity = (RigidBodyEntity*)*it;
		auto rigidBody = entity->GetBody();

		entities.erase(it);
		world->removeRigidBody(rigidBody);

		delete entity->GetBody();
		delete entity;
	}
	return false;
}

void Scene::SetLayerCollision(uint64_t ID0, uint64_t ID1, bool bEnableCollision)
{
	if (ID0 > nLayeCollisionMatrixRows - 1 || ID1 > nLayeCollisionMatrixRows - 1)
	{
		// Reallocate larger matrix
		uint64_t nRows = std::max(ID0, ID1) + 1;
		layeCollisionMatrix.resize(nRows * nRows);

		// Move old datas to correct places
		// i = 0 will not run, cuz 0th will remain good in memory
		for (uint64_t i = nLayeCollisionMatrixRows - 1; i > 0; i--)
		{
			uint8_t* src = i * nLayeCollisionMatrixRows + (uint8_t*)layeCollisionMatrix.data();
			uint8_t* dst = src + i * (nRows - nLayeCollisionMatrixRows);
			memmove(dst, src, nLayeCollisionMatrixRows);

			// Set newly allocated bytes to 1 (part0)
			// [src, dst[ set 1 these byte are the newly allocated ones, ID0 can collide with everything, and ID1 also
			memset(src, 1, dst - src);
		}

		// Set newly allocated bytes to 1 (part1)
		uint64_t asd = nRows + (nRows - nLayeCollisionMatrixRows);
		memset((uint8_t*)layeCollisionMatrix.data() + (nRows *  nRows) - asd, 1, asd);

		nLayeCollisionMatrixRows = nRows;
	}

	layeCollisionMatrix[ID0 + nLayeCollisionMatrixRows * ID1] = bEnableCollision;
	layeCollisionMatrix[ID1 + nLayeCollisionMatrixRows * ID0] = bEnableCollision;
}

bool Scene::GetDebugData(Vec3*& linesFromNonUniqPoints_out, size_t& nLines_out) const
{
	BulletDebugDraw* debugInfoGatherer = (BulletDebugDraw*)world->getDebugDrawer();
	if (debugInfoGatherer)
	{
		debugInfoGatherer->GetDebugData(linesFromNonUniqPoints_out, nLines_out);
		return true;
	}

	return false;
}

}