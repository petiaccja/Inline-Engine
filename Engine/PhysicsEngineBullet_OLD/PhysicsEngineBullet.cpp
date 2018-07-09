#include "PhysicsEngineBullet.hpp"

namespace inl::physics::bullet {

PhysicsEngineBullet::PhysicsEngineBullet()
{

}

PhysicsEngineBullet::~PhysicsEngineBullet()
{
	for (Scene* a : scenes)
		delete a;
}

void PhysicsEngineBullet::Update(float deltaTime)
{
	for (auto& scene : scenes)
	{
		scene->Update(deltaTime);
	}
}

Scene* PhysicsEngineBullet::CreateScene()
{
	Scene* p = new Scene();
	scenes.push_back(p);
	return p;
}

bool PhysicsEngineBullet::RemoveScene(Scene* scene)
{
	auto it = std::find(scenes.begin(), scenes.end(), scene);

	if (it != scenes.end())
	{
		scenes.erase(it);
		return true;
	}

	return false;
}

}