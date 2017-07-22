#pragma once

#include "PhysicsEngine/IPhysicsEngine.hpp"
#include "Scene.hpp"

using namespace inl;
using namespace inl::physics::bullet;

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