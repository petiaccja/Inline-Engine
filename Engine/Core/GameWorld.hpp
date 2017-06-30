#pragma once
#include "EngineCore.hpp"
#include "MeshComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "PerspectiveCameraComponent.hpp"
#include "Transform3DComponent.hpp"
#include "WorldComponent.hpp"
#include "Script.hpp"

class Script;

class GameWorld
{
public:
	template<class ScriptClass>
	inline Script* AddScript();

	inline Actor* AddActor();
	// Actor* AddActor(WorldComponent* rootComp);
	inline Actor* AddActor(const std::string& modelFilePath, float mass = 0);
	inline Actor* AddActor_Mesh(const std::string& modelFilePath);
	inline Actor* AddActor_RigidBody(const std::string& modelFilePath, float mass = 0);
	inline Actor* AddActor_RigidBodyCapsule(float height, float radius, float mass = 0);
	inline Actor* AddActor_Camera();

	inline MeshComponent*		 AddComponent_Mesh(const std::string& modelFilePath);
	inline RigidBodyComponent*	 AddComponent_RigidBody(const std::string& modelFilePath, float mass = 0);
	inline RigidBodyComponent*	 AddComponent_RigidBodyCapsule(float height, float radius, float mass = 0);
	inline PerspectiveCameraComponent*		 AddComponent_Camera();
	inline Transform3DComponent* AddComponent_Transform3D();
	
	inline void Remove(Actor* a);
	inline void Remove(WorldComponent* c);

	inline void SetCam(PerspectiveCameraComponent* c);

	inline bool TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceInfo_out);
};
extern GameWorld World;

template<class ScriptClass>
Script* GameWorld::AddScript() 
{ 
	return Core.AddScript<ScriptClass>(); 
}

Actor* GameWorld::AddActor() 
{ 
	return Core.AddActor();
}

Actor* GameWorld::AddActor_Mesh(const std::string& modelFilePath) 
{ 
	return Core.AddActor_Mesh(modelFilePath);
}

Actor* GameWorld::AddActor(const std::string& modelFilePath, float mass /*= 0*/)
{
	return Core.AddActor(modelFilePath, mass);
}

Actor* GameWorld::AddActor_RigidBody(const std::string& modelFilePath, float mass /*= 0*/)
{
	return Core.AddActor_RigidBody(modelFilePath, mass);
}

Actor* GameWorld::AddActor_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	return Core.AddActor_RigidBodyCapsule(height, radius, mass);
}

Actor* GameWorld::AddActor_Camera()
{
	return Core.AddActor_Camera();
}

MeshComponent* GameWorld::AddComponent_Mesh(const std::string& modelFilePath)
{
	return Core.AddComponent_Mesh(modelFilePath);
}

RigidBodyComponent* GameWorld::AddComponent_RigidBody(const std::string& modelFilePath, float mass)
{
	return Core.AddComponent_RigidBody(modelFilePath, mass);
}

RigidBodyComponent* GameWorld::AddComponent_RigidBodyCapsule(float height, float radius, float mass /*= 0*/) 
{ 
	return Core.AddComponent_RigidBodyCapsule(height, radius, mass);
}
PerspectiveCameraComponent* GameWorld::AddComponent_Camera()
{
	return Core.AddComponent_Camera();
}

Transform3DComponent* GameWorld::AddComponent_Transform3D()
{
	return Core.AddComponent_Transform3D();
}

void GameWorld::Remove(Actor* a)
{
	return Core.RemoveActor(a);
}

void GameWorld::Remove(WorldComponent* c)
{
	return Core.RemoveComponent(c);
}

void GameWorld::SetCam(PerspectiveCameraComponent* c)
{
	return Core.SetCam(c);
}

bool GameWorld::TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceInfo_out)
{
	return Core.TraceClosestPoint_Physics(from, to, traceInfo_out);
}