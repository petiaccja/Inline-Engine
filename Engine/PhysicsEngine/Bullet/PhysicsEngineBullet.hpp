#pragma once
#include "PhysicsEngine/IPhysicsEngine.hpp"
#include "Scene.hpp"

namespace inl::physics::bullet {

class PhysicsEngineBullet
{
public:
	PhysicsEngineBullet();
	~PhysicsEngineBullet();

	void Update(float deltaTime);

	Scene* CreateScene();

	bool RemoveScene(Scene* scene);

protected:
	std::vector<Scene*> scenes;

};

} // namespace inl::physics::bullet