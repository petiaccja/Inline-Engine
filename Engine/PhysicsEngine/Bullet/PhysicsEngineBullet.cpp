#include "PhysicsEngineBullet.hpp"

#include "RigidBodyEntity.hpp"
#include "SoftBodyEntity.hpp"
#include "PhysicsEngineBulletDebugGatherer.hpp"

#include <Bullet/btBulletDynamicsCommon.h>
#include <Bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <Bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <Bullet/BulletSoftBody/btSoftBodyHelpers.h>
#include <Bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>


PhysicsEngineBullet::PhysicsEngineBullet(const PhysicsEngineBulletDesc& d) 
{
	world = new btDiscreteDynamicsWorld(new	BulletCollisionDispatcher(new btDefaultCollisionConfiguration, this),
										new btDbvtBroadphase,
										new btSequentialImpulseConstraintSolver,
										new btDefaultCollisionConfiguration);

	//world->getSolverInfo().m_numIterations = 4;
	//world->getSolverInfo().m_solverMode = SOLVER_SIMD;
	//world->getDispatchInfo().m_enableSPU = true;

	world->setGravity(btVector3(d.gravity.x, d.gravity.y, d.gravity.z));

	btIDebugDraw* debugDrawer = (btIDebugDraw*)new PhysicsEngineBulletDebugGatherer();
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb);

	//world->setDebugDrawer(debugDrawer);

	// Populate collisionMatrix with true values, everything can collide with everything by default
	nLayeCollisionMatrixRows = 16; // Start with 16x16 matrix
	layeCollisionMatrix.resize(nLayeCollisionMatrixRows * nLayeCollisionMatrixRows);
	memset(layeCollisionMatrix.data(), 1, nLayeCollisionMatrixRows * nLayeCollisionMatrixRows);
}

PhysicsEngineBullet::~PhysicsEngineBullet()
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

void PhysicsEngineBullet::Release()
{
	delete this;
}

void PhysicsEngineBullet::Update(float deltaTime)
{
	{
		//PROFILE_SCOPE("Simulate");
		world->stepSimulation(deltaTime);
	}
	
	{
		//PROFILE_SCOPE("Contact List Query");
		contactList.clear();

		int numManifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; i++)
		{
			btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);

			int numContacts = contactManifold->getNumContacts();
			if (numContacts != 0)
			{
				const btCollisionObject* colA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
				const btCollisionObject* colB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

				// Fill up our structure with contact informations
				physics::Collision colInfo;
				
				if (!colA->getCollisionShape()->isSoftBody())
				{
					colInfo.rigidBodyA = (RigidBodyEntity*)colA->getUserPointer();
					colInfo.softBodyA = nullptr;
				}
				else
				{
					colInfo.softBodyA = (SoftBodyEntity*)colA->getUserPointer();
					colInfo.rigidBodyA = nullptr;
				}
					

				if (!colB->getCollisionShape()->isSoftBody())
				{
					colInfo.rigidBodyB = (RigidBodyEntity*)colB->getUserPointer();
					colInfo.softBodyB = nullptr;
				}
				else
				{
					colInfo.softBodyB = (SoftBodyEntity*)colB->getUserPointer();
					colInfo.rigidBodyB = nullptr;
				}

				for (int j = 0; j < numContacts; j++)
				{
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					if (pt.getDistance() <= 0.f)
					{
						//Fill contact data
						physics::ContactPoint c;
						c.normalA = -Vec3(pt.m_normalWorldOnB.x(), pt.m_normalWorldOnB.y(), pt.m_normalWorldOnB.z());
						c.normalB = Vec3(pt.m_normalWorldOnB.x(), pt.m_normalWorldOnB.y(), pt.m_normalWorldOnB.z());
						c.posA = Vec3(pt.m_positionWorldOnA.x(), pt.m_positionWorldOnA.y(), pt.m_positionWorldOnA.z());
						c.posB = Vec3(pt.m_positionWorldOnB.x(), pt.m_positionWorldOnB.y(), pt.m_positionWorldOnB.z());
						colInfo.contacts.push_back(c);

						const btVector3& ptA = pt.getPositionWorldOnA();
						const btVector3& ptB = pt.getPositionWorldOnB();
						const btVector3& normalOnB = pt.m_normalWorldOnB;
					}
				}

				if (colInfo.contacts.size() != 0)
					contactList.push_back(colInfo);
			}
		}
	}

	if (world->getDebugDrawer())
	{
		((PhysicsEngineBulletDebugGatherer*)world->getDebugDrawer())->ClearFrameData();
		world->debugDrawWorld();
	}
}

bool PhysicsEngineBullet::TraceClosestPoint(const Vec3& from, const Vec3& to, physics::TraceResult& traceInfo_out, const physics::TraceParams& params /*= physics::TraceParams()*/)
{
	ClosestRayCallback callb(this, { from.x, from.y, from.z }, { to.x, to.y, to.z }, params.ignoredCollisionLayers);

	world->rayTest({ from.x, from.y, from.z }, { to.x, to.y, to.z }, callb);

	if (callb.hasHit())
	{
		traceInfo_out.normal = Vec3(callb.m_hitNormalWorld.x(), callb.m_hitNormalWorld.y(), callb.m_hitNormalWorld.z());
		traceInfo_out.pos = Vec3(callb.m_hitPointWorld.x(), callb.m_hitPointWorld.y(), callb.m_hitPointWorld.z());
		traceInfo_out.userPointer = callb.m_collisionObject->getUserPointer();
		return true;
	}
	else
	{
		return false;
	}
}

physics::IRigidBodyEntity* PhysicsEngineBullet::AddEntityRigidDynamic(Vec3* vertices, uint32_t nVertices, float mass /*= 1*/) 
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

physics::IRigidBodyEntity* PhysicsEngineBullet::AddEntityRigidStatic(Vec3* vertices, uint32_t nVertices, void* indices, uint32_t indexSize, uint32_t nIndices) 
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
			myIndices[i] = (int)*((uint32_t*)(indices) + i);

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

physics::IRigidBodyEntity* PhysicsEngineBullet::AddEntityRigidCapsule(float height, float radius, float mass)
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

bool PhysicsEngineBullet::RemoveEntity(physics::IRigidBodyEntity* e)
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

void PhysicsEngineBullet::SetLayerCollision(size_t ID0, size_t ID1, bool bEnableCollision)
{
	if (ID0 > nLayeCollisionMatrixRows - 1 || ID1 > nLayeCollisionMatrixRows - 1)
	{
		// Reallocate larger matrix
		size_t nRows = std::max(ID0, ID1) + 1;
		layeCollisionMatrix.resize(nRows * nRows);

		// Move old datas to correct places
		// i = 0 will not run, cuz 0th will remain good in memory
		for (size_t i = nLayeCollisionMatrixRows - 1; i > 0; i--)
		{
			uint8_t* src = i * nLayeCollisionMatrixRows + (uint8_t*)layeCollisionMatrix.data();
			uint8_t* dst = src + i * (nRows - nLayeCollisionMatrixRows);
			memmove(dst, src, nLayeCollisionMatrixRows);

			// Set newly allocated bytes to 1 (part0)
			// [src, dst[ set 1 these byte are the newly allocated ones, ID0 can collide with everything, and ID1 also
			memset(src, 1, dst - src);
		}

		// Set newly allocated bytes to 1 (part1)
		size_t asd = nRows + (nRows - nLayeCollisionMatrixRows);
		memset((uint8_t*)layeCollisionMatrix.data() + (nRows *  nRows) - asd, 1, asd);

		nLayeCollisionMatrixRows = nRows;
	}

	layeCollisionMatrix[ID0 + nLayeCollisionMatrixRows * ID1] = bEnableCollision;
	layeCollisionMatrix[ID1 + nLayeCollisionMatrixRows * ID0] = bEnableCollision;
}

bool PhysicsEngineBullet::GetDebugData(Vec3*& linesFromNonUniqPoints_out, size_t& nLines_out) const
{
	PhysicsEngineBulletDebugGatherer* debugInfoGatherer = (PhysicsEngineBulletDebugGatherer*)world->getDebugDrawer();
	if (debugInfoGatherer)
	{
		debugInfoGatherer->GetDebugData(linesFromNonUniqPoints_out, nLines_out);
		return true;
	}
	
	return false;
}