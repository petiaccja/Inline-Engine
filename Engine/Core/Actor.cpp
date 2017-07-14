#include "Actor.hpp"
#include "SceneScript.hpp"
#include "Scene.hpp"

using namespace inl::core;

Actor::Actor()
:Actor(TRANSFORM)
{
}

Actor::Actor(ePartType type)
:Part(type)
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