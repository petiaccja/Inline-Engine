#pragma once


#include <memory>
#include <InlineMath.hpp>

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>

#include <unordered_set>


namespace inl::pxeng_bl {

class RigidBody;


class Scene {
public:
	Scene();
	~Scene() = default;

	void Update(float elapsed);

	void SetGravity(const Vec3& gravity);
	Vec3 GetGravity() const;

	void AddEntity(const RigidBody* entity);
	void RemoveEntity(const RigidBody* entity);
private:
	std::unique_ptr<btBroadphaseInterface> m_broadphase;
	std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> m_collisionDispatcher;
	std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
	std::unique_ptr<btDiscreteDynamicsWorld> m_world;

	std::unordered_set<const RigidBody*> m_entities;
};


} // namespace inl::pxeng_bl
