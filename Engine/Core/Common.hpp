#pragma once
#include <PhysicsEngine\Common.hpp>
#include <functional>
#include <vector>
#include <InlineMath.hpp>

namespace inl::core {


class Actor;
class RigidBodyPart;
class SoftBodyPart;
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

struct PartCollision
{
	// TODO make base class from RigidBody and SoftBody

	// self RigidBody colliding, otherwise nullptr
	RigidBodyPart* rigidBodyA;

	// other RigidBody colliding, otherwise nullptr
	RigidBodyPart* rigidBodyB;

	// self SoftBody colliding, otherwise nullptr
	SoftBodyPart* softBodyA;

	// other SoftBody colliding, otherwise nullptr
	SoftBodyPart* softBodyB;

	// Contact points generated between bodyA and bodyB
	std::vector<physics::Contact> contacts;
};

struct Collision
{
	Actor*  actorA;
	Actor*  actorB;

	// All collisions between actorA and actorB in that frame, it contains "Current bodyA bodyB collision" too
	std::vector<PartCollision> partCollisions;
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

} // namespace inl::core