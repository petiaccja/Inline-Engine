#pragma once


#include "PhysicsEngine.hpp"

#include "Scene.hpp"
#include "RigidBody.hpp"
#include "MeshShape.hpp"


namespace inl::pxeng_bl {



Scene* PhysicsEngine::CreateScene() const {
	return new Scene();
}


RigidBody* PhysicsEngine::CreateRigidBody() const {
	return new RigidBody();
}

MeshShape* PhysicsEngine::CreateMeshShape() const {
	return new MeshShape();
}


} // namespace inl::pxeng_bl