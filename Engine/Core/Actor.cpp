#include "Actor.hpp"
#include "SceneScript.hpp"
#include "Scene.hpp"

namespace inl::core {

Actor::Actor(Scene* scene)
:Actor(scene, TRANSFORM)
{
}

Actor::Actor(Scene* scene, ePartType type)
:Part(scene, type)
{

}

void Actor::Update(float deltaTime)
{
	onUpdate(deltaTime);
}

void Actor::AddBehavior(ActorBehavior* b)
{
	behaviors.push_back(b);
}

MeshActor* Actor::AddActor_Mesh(const path& modelPath)
{
	MeshActor* childActor = scene->AddActor_Mesh(modelPath);
	Attach(childActor);
	return childActor;
}

}