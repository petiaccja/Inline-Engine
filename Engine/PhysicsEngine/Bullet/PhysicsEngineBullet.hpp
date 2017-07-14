#pragma once

#include "PhysicsEngine/IPhysicsEngine.hpp"
#include "Scene.hpp"

using namespace inl;
using namespace inl::physics::bullet;

namespace inl::physics::bullet {

class PhysicsEngineBullet : public IPhysicsEngine
{
public:
	PhysicsEngineBullet();
	~PhysicsEngineBullet();

	void Release() override;
	void Update(float deltaTime) override;

	Scene* AddScene();
	bool RemoveScene(Scene* scene);

protected:
	std::vector<Scene*> scenes;

};

} // namespace inl::physics::bullet