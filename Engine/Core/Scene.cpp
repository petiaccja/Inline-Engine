#include "Scene.hpp"

namespace inl::core {

Scene::~Scene()
{
	for (Actor* a : actors)
		delete a;
}

Actor* Scene::AddActor()
{
	return nullptr;// Core.AddActor();
}

Actor* Scene::AddActor_Mesh(const std::string& modelFilePath)
{
	return nullptr;// return Core.AddActor_Mesh(modelFilePath);
}

Actor* Scene::AddActor(const std::string& modelFilePath, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor(modelFilePath, mass);
}

Actor* Scene::AddActor_RigidBody(const std::string& modelFilePath, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor_RigidBody(modelFilePath, mass);
}

RigidBodyActor* Scene::AddActor_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor_RigidBodyCapsule(height, radius, mass);
}

PerspCameraActor* Scene::AddActor_PerspCamera()
{
	gxeng::PerspectiveCamera* cam = graphicsEngine->CreatePerspectiveCamera("WorldCam"); // For now hardcoded to "WorldCam", graphicsEngine check for this exact name (HEKK)
	PerspCameraActor* a = new PerspCameraActor(cam);
	actors.push_back(a);
	return a;
}

MeshPart* Scene::AddPart_Mesh(const std::string& modelFilePath)
{
	return nullptr;//return Core.AddPart_Mesh(modelFilePath);
}

RigidBodyPart* Scene::AddPart_RigidBody(const std::string& modelFilePath, float mass)
{
	return nullptr;//return Core.AddPart_RigidBody(modelFilePath, mass);
}

RigidBodyPart* Scene::AddPart_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	return nullptr;//return Core.AddPart_RigidBodyCapsule(height, radius, mass);
}
PerspCameraPart* Scene::AddPart_Camera()
{
	return nullptr;//return Core.AddPart_Camera();
}


void Scene::RemoveActor(Actor* a)
{
	return;// Core.RemoveActor(a);
}

void Scene::RemovePart(Part* c)
{
	return;// Core.RemovePart(c);
}

void Scene::SetCam(Actor* c)
{
	assert(c->Is<PerspCameraPart>());
	assert(0);
}

void Scene::SetCam(PerspCameraPart* c)
{
	assert(0);
	//assert(defaultGraphicsScene);
	//defaultGraphicsScene->SetCamera(c->GetCam());
}

bool Scene::TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceInfo_out)
{
	return false;// Core.TraceClosestPoint_Physics(from, to, traceInfo_out);
}

void Scene::SetLayerCollision(uint64_t ID0, uint64_t ID1, bool bEnableCollision)
{
	physicsScene->SetLayerCollision(ID0, ID1, bEnableCollision);
	//assert(physicsEngine);
	//physicsEngine->SetLayerCollision(ID0, ID1, bEnableCollision);
}

} // namespace inl::core