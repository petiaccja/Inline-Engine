#pragma once
#include <Core\ActorScript.hpp>
#include <Core\PerspCameraPart.hpp>
#include <Core\RigidBodyPart.hpp>
#include <Core\ActorScript.hpp>
#include <Core\MeshPart.hpp>
#include <Core\Actors.hpp>
#include <InlineMath.hpp>

using namespace inl::core;

enum eES_CollisionGroup : uint64_t
{
	PLAYER,
	SHELL,
	GROUND,
	BULLET,
};

class PlayerScript : public ActorScript
{
public:
	PlayerScript();

	void Update(float deltaSeconds);

protected:
	// Components
	PerspCameraPart* camComp;

	MeshPart* ak47Graphics;

	RigidBodyActor* playerCapsule;

	// Moving
	bool bMovingFront;
	bool bMovingBack;
	bool bMovingLeft;
	bool bMovingRight;
	
	float playerMoveSpeed;
	float playerMaxMoveSpeed;

	// Jump
	bool bCanJump;

	// Cam roting
	float pixelsToRot360;

	Vec2i mousePosWhenPress;

	//sound::IEmitter* walkSound;
	//sound::IEmitter* gunSound;
	//sound::IEmitter* shellSound;

	//W, S, A, D down count
	size_t nButtonsDown;

	float shootTimer;
	float rateOfFire;

	bool bSquatting;
	core::Collision playeCollisionData;
};