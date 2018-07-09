#pragma once


namespace inl::pxeng_bl {

class Scene;
class RigidBody;
class MeshShape;


class PhysicsEngine {
public:
	Scene* CreateScene() const;

	RigidBody* CreateRigidBody() const;

	MeshShape* CreateMeshShape() const;
};


} // namespace inl::pxeng_bl