#pragma once
// Behaviors are groups of {actorScript0, actorScript1, ...} which control Actor
#include "ActorScript.hpp"
#include <vector>

class Behavior
{
public:
	 void AddScript(ActorScript* s)
	{
		entityScripts.push_back(s);
	}

	 void SetActor(Actor* a)
	{
		for (auto& s : entityScripts)
			s->SetActor(a);
	}

	 std::vector<ActorScript*> GetScripts() {return entityScripts;}

protected:
	std::vector<ActorScript*> entityScripts;
};