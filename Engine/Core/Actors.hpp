#pragma once
#include "Actor.hpp"
#include "RigidBodyPart.hpp"
#include "PerspCameraPart.hpp"
#include "DirectionalLightPart.hpp"
#include "MeshPart.hpp"
#include "TransformPart.hpp"

namespace inl::core {

class RigidBodyActor : public Actor, public RigidBodyPart
{
public:
	RigidBodyActor(Scene* scene, physics::IRigidBodyEntity* a)
	:Actor(scene, RIGID_BODY), RigidBodyPart(scene, a), Part(scene, RIGID_BODY)
	{

	}
};

class MeshActor : public Actor, public MeshPart
{
public:
	MeshActor(Scene* scene, gxeng::MeshEntity* entity)
	:Actor(scene, MESH), MeshPart(scene, entity), Part(scene, MESH)
	{

	}
};

class PerspCameraActor : public Actor, public PerspCameraPart
{
public:
	PerspCameraActor(Scene* scene, gxeng::PerspectiveCamera* cam)
	:Actor(scene, CAMERA), PerspCameraPart(scene, cam), Part(scene, CAMERA)
	{

	}
};

class DirectionalLightActor : public Actor, public DirectionalLightPart
{
public:
	DirectionalLightActor(Scene* scene, gxeng::Scene* gxScene)
	:Actor(scene, DIRECTIONAL_LIGHT), DirectionalLightPart(scene, gxScene), Part(scene, DIRECTIONAL_LIGHT)
	{
		
	}
};

class EmptyActor : public Actor, public TransformPart
{
public:
	EmptyActor(Scene* scene)
	:Actor(scene, TRANSFORM), TransformPart(scene), Part(scene, TRANSFORM)
	{

	}
};



} // namespace inl::core