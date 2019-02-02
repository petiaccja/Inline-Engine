#include "Scene.hpp"
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include "BulletTypes.hpp"
#include "BaseLibrary/Exception/Exception.hpp"

#include "RigidBody.hpp"

#undef GetObject // faszom kivan ezzel a kurva winapival


namespace inl::pxeng_bl {


Scene::Scene() {
	m_broadphase.reset(new btDbvtBroadphase());
	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_collisionDispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration.get()));
	m_solver.reset(new btSequentialImpulseConstraintSolver());
	m_world.reset(new btDiscreteDynamicsWorld(
		m_collisionDispatcher.get(),
		m_broadphase.get(),
		m_solver.get(),
		m_collisionConfiguration.get()));

	
}


void Scene::Update(float elapsed) {
	m_world->stepSimulation(elapsed, 10);
}


void Scene::SetGravity(const Vec3& gravity) {
	m_world->setGravity(Conv(gravity));
}


Vec3 Scene::GetGravity() const {
	return Conv(m_world->getGravity());
}


void Scene::AddEntity(const RigidBody* entity) {
	auto [it, isNew] = m_entities.insert(entity);
	if (isNew) {
		m_world->addRigidBody(entity->GetObject());
	}
	else {
		throw InvalidArgumentException("Entity already added to scene.");
	}
}

void Scene::RemoveEntity(const RigidBody* entity) {
	auto it = m_entities.find(entity);
	if (it != m_entities.end()) {
		m_entities.erase(it);
	}
	throw InvalidArgumentException("Entity is not part of scene.");
}


} // namespace inl::pxeng_bl