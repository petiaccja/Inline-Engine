#pragma once
#include "ActorScript.hpp"
#include <vector>

namespace inl::core {

// Behaviors are groups of {actorScript0, actorScript1, ...} which drives Actor gameplay logic
class ActorBehavior
{
public:
	void AddScript(ActorScript* s);
	void SetActor(Actor* a);

	std::vector<ActorScript*>& GetScripts();

protected:
	std::vector<ActorScript*> entityScripts;
};

} // namespace inl::core