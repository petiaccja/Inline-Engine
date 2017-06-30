#pragma once
#include "SoftBodyComponent.hpp"
#include "RigidBodyComponent.hpp"
//#include "SoundEngine/ISoundData.h"
//#include "SoundEngine/IEmitter.h"
#include <functional>


namespace inl::core {

//struct MonoSound
//{
//	sound::ISoundData* soundData;
//	sound::IEmitter* soundEmitter;
//};

struct Task
{
	std::function<void()> callb;
	float timeLeft;
};

struct ComponentCollision
{
	// self RigidBody colliding, otherwise nullptr
	RigidBodyComponent* rigidBodyA;

	// other RigidBody colliding, otherwise nullptr
	RigidBodyComponent* rigidBodyB;

	// self SoftBody colliding, otherwise nullptr
	SoftBodyComponent* softBodyA;

	// other SoftBody colliding, otherwise nullptr
	SoftBodyComponent* softBodyB;

	// Contact points generated between bodyA and bodyB
	std::vector<physics::ContactPoint> contacts;
};

struct Collision
{
	Actor*  actorA;
	Actor*  actorB;

	// All collisions between actorA and actorB in that frame, it contains "Current bodyA bodyB collision" too
	std::vector<ComponentCollision> collisions;

	// self RigidBody colliding, otherwise nullptr
	RigidBodyComponent* rigidBodyA;

	// other RigidBody colliding, otherwise nullptr
	RigidBodyComponent* rigidBodyB;

	// self SoftBody colliding, otherwise nullptr
	SoftBodyComponent* softBodyA;

	// other SoftBody colliding, otherwise nullptr
	SoftBodyComponent* softBodyB;

	// Contact points generated between bodyA and bodyB
	std::vector<physics::ContactPoint> contacts;
};

struct ContactPoint : public inl::physics::ContactPoint
{

};

struct PhysicsTraceResult
{
	Actor* actor;

	Vec3 pos;
	Vec3 normal;
};

struct PhysicsTraceParams : public physics::TraceParams
{
};

} // namespace inl