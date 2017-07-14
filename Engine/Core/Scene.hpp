#pragma once
#include "SceneScript.hpp"
#include "Common.hpp"

#include "Actors.hpp"

#include <GraphicsEngine_LL\GraphicsEngine.hpp>
#include <GraphicsEngine_LL\Scene.hpp>
#include <PhysicsEngine\Bullet\Scene.hpp>

namespace inl::core {

using namespace inl;

class MeshPart;
class RigidBodyPart;
class PerspCameraPart;

class Scene
{
public:
	~Scene();

	template<class T>
	SceneScript* AddScript() {return return Core.AddScript<T>();}

	Actor*				AddActor();
	Actor*				AddActor(const std::string& modelFilePath, float mass = 0);
	Actor*				AddActor_Mesh(const std::string& modelFilePath);
	Actor*				AddActor_RigidBody(const std::string& modelFilePath, float mass = 0);
	RigidBodyActor*		AddActor_RigidBodyCapsule(float height, float radius, float mass = 0);
	PerspCameraActor*	AddActor_PerspCamera();

	MeshPart*			AddPart_Mesh(const std::string& modelFilePath);
	RigidBodyPart*		AddPart_RigidBody(const std::string& modelFilePath, float mass = 0);
	RigidBodyPart*		AddPart_RigidBodyCapsule(float height, float radius, float mass = 0);
	PerspCameraPart*	AddPart_Camera();

	void SetLayerCollision(uint64_t ID0, uint64_t ID1, bool bEnableCollision);

	void SetCam(Actor* c);
	void SetCam(PerspCameraPart* c);

	inline void RemoveActor(Actor* a);
	inline void RemovePart(Part* c);

	bool TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceInfo_out);

protected:
	// Graphics scene
	gxeng::GraphicsEngine* graphicsEngine;
	gxeng::Scene* graphicsScene;

	// Physics scene (TODO Interface for physics scene)
	physics::bullet::Scene* physicsScene;

	// Sound scene

	// Actors
	std::vector<Actor*> actors;
};

} // namespace inl::core