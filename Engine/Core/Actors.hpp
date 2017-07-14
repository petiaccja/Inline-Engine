#pragma once
#include "Actor.hpp"
#include "RigidBodyPart.hpp"
#include "PerspCameraPart.hpp"

namespace inl::core {

class RigidBodyActor : public Actor, public RigidBodyPart
{
public:
	RigidBodyActor(physics::IRigidBodyEntity* a)
	:Actor(RIGID_BODY), RigidBodyPart(a), Part(ePartType::RIGID_BODY)
	{

	}
};

class PerspCameraActor : public Actor, public PerspCameraPart
{
public:
	PerspCameraActor(gxeng::PerspectiveCamera* cam)
	:Actor(CAMERA), PerspCameraPart(cam), Part(CAMERA)
	{

	}
};

} // namespace inl::core