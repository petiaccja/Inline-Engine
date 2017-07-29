#pragma once
#include "Actor.hpp"
#include "RigidBodyPart.hpp"
#include "PerspCameraPart.hpp"
#include "DirectionalLightPart.hpp"
#include "MeshPart.hpp"

namespace inl::core {

class RigidBodyActor : public Actor, public RigidBodyPart
{
public:
	RigidBodyActor(physics::IRigidBodyEntity* a)
	:Actor(RIGID_BODY), RigidBodyPart(a), Part(RIGID_BODY)
	{

	}
};

class MeshActor : public Actor, public MeshPart
{
public:
	MeshActor(gxeng::MeshEntity* entity)
	:Actor(MESH), MeshPart(entity), Part(MESH)
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

class DirectionalLightActor : public Actor, public DirectionalLightPart
{
public:
	DirectionalLightActor(gxeng::Scene* scene)
	:Actor(CAMERA), DirectionalLightPart(scene), Part(DIRECTIONAL_LIGHT)
	{
		
	}
};

} // namespace inl::core