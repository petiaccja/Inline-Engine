#pragma once
#include "SceneScript.hpp"
#include "Common.hpp"

#include "Actors.hpp"

#include <GraphicsEngine_LL\GraphicsEngine.hpp>
#include <GraphicsEngine_LL\Scene.hpp>
#include <PhysicsEngineBullet\Scene.hpp>

// TMP !!
#include <GraphicsEngine_LL\DirectionalLight.hpp>
#include <filesystem>


namespace inl::core {

class MeshPart;
class RigidBodyPart;
class PerspCameraPart;
class Core;
class Scene
{
public:
	Scene(Core* core);
	~Scene();

	void Update(float deltaTime);

	bool TraceGraphicsRay(const Ray3D& ray, TraceResult& traceResult_out);

	template<class T>
	SceneScript* AddScript() { return nullptr; /* core.AddScript<T>(); */ }

	EmptyActor*				AddActor();
	void					AddActor(Actor* a);
	MeshActor*				AddActor_Mesh(const std::experimental::filesystem::path& modelPath);
	RigidBodyActor*			AddActor_RigidBody(const std::experimental::filesystem::path& modelPath, float mass = 0);
	RigidBodyActor*			AddActor_RigidBodyCapsule(float height, float radius, float mass = 0);
	PerspCameraActor*		AddActor_PerspCamera();
	DirectionalLightActor*	AddActor_DirectionalLight();

	MeshPart*			CreatePart_Mesh(const std::string& modelFilePath);
	RigidBodyPart*		CreatePart_RigidBody(const std::string& modelFilePath, float mass = 0);
	RigidBodyPart*		CreatePart_RigidBodyCapsule(float height, float radius, float mass = 0);
	PerspCameraPart*	CreatePart_Camera();

	void SetLayerCollision(uint64_t ID0, uint64_t ID1, bool bEnableCollision);

	void SetCam(Actor* c);
	void SetCam(PerspCameraPart* c);

	inline void RemoveActor(Actor* a);
	inline void DestroyPart(Part* c);

	//bool TracePhysicsRay(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceInfo_out);
	//bool TraceClosestPoint_Physics(PhysicsTraceResult& traceInfo_out);

protected:
	// Core
	Core* core;

	// Graphics scene
	gxeng::Scene* graphicsScene;

	// Physics scene
	physics::bullet::Scene* physicsScene;

	// Actors
	std::vector<Actor*> actors;

	std::vector<Part*> parts;

	// TMP REMOVE
	inl::gxeng::DirectionalLight* sun;
};

} // namespace inl::core