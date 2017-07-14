#include "ActorBehavior.hpp"

using namespace inl::core;

void ActorBehavior::AddScript(ActorScript* s)
{
	entityScripts.push_back(s);
}

void ActorBehavior::SetActor(Actor* a)
{
	for (auto& s : entityScripts)
		s->SetActor(a);
}

std::vector<ActorScript*>& ActorBehavior::GetScripts()
{
	return entityScripts;
}