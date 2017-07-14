#pragma once

namespace inl::core {

class Actor;

class ActorScript
{
public:
	ActorScript();

public:
	virtual void Update(float deltaTime){}

	 void SetActor(Actor* a) { actor = a; }

	 Actor* GetActor() {return actor;}

protected:
	Actor* actor;
};

}