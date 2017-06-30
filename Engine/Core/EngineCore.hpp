#pragma once
// The purpose of this class is to take the most minimal input from the user to (startup, use) the engine

#include "ActorScript.hpp"
#include "MeshComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "PerspectiveCameraComponent.hpp"
#include "Transform3DComponent.hpp"
#include "Common.hpp"

#include <GuiEngine/GuiEngine.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine/Definitions.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <PhysicsEngine/IPhysicsEngine.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <PhysicsEngine/Bullet/PhysicsEngineBullet.hpp>

#include <unordered_map>
#include <vector>
#include <functional>

class Script;

using namespace inl::core;
using namespace inl::gui;
using namespace inl::physics;
using namespace inl::physics::bullet;
//using namespace inl::gxeng;
using namespace inl::gxapi;
using namespace inl::gxapi_dx12;

class EngineCore
{
public:
	EngineCore();
	~EngineCore();

	gxeng::GraphicsEngine* InitGraphicsEngine(int width, int height, HWND hwnd);
	IPhysicsEngine* InitPhysicsEngineBullet(const PhysicsEngineBulletDesc& d = PhysicsEngineBulletDesc());
	GuiEngine*		InitGuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow);

	bool TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceResult_out, const PhysicsTraceParams& params = PhysicsTraceParams());

	// Todo
	//bool TraceClosestPoint_Graphics(const Vec3& from, const Vec3& to, TraceResult& traceInfo_out);
	
	//sound::IEmitter* CreateMonoSound(const std::string& filePath, float volumeNormedPercent = 1, bool bLoop = false);
	
	Actor* AddActor();
	Actor* AddActor(const std::string& modelFilePath, float mass = 0);
	Actor* AddActor(WorldComponent* rootComp);
	Actor* AddActor_Mesh(const std::string& modelFilePath);
	Actor* AddActor_RigidBody(const std::string& modelFilePath, float mass);
	Actor* AddActor_RigidBodyCapsule(float height, float radius, float mass = 0);
	Actor* AddActor_Camera();
	
	void RemoveActor(Actor* a);
	void RemoveComponent(WorldComponent* c);

	void AddTask(const std::function<void()>& callb, float timeToProceed);
	
	template<class ScriptClass>
	Script* AddScript()
	{
		ScriptClass* p = new ScriptClass();
		scripts.push_back(p);
		return p;
	}
	
	template<class ActorScriptClass>
	ActorScript* AddActorScript()
	{
		ActorScriptClass* p = new ActorScriptClass();
		p->SetEntity(Core.AddEntity());
	
		entityScripts.push_back(p);
		return p;
	}
	
	MeshComponent*			AddComponent_Mesh(const std::string& modelAssetPath);
	RigidBodyComponent*		AddComponent_RigidBody(const std::string& modelAssetPath, float mass);
	RigidBodyComponent*		AddComponent_RigidBodyCapsule(float height, float radius, float mass = 0);
	PerspectiveCameraComponent*		AddComponent_Camera();
	Transform3DComponent*	AddComponent_Transform3D();
	
	void SetLayerCollision(size_t ID0, size_t ID1, bool bEnableCollision);
	
	void SetCam(PerspectiveCameraComponent* c)
	{ 
		assert(0);
		//assert(defaultGraphicsScene); 
		//defaultGraphicsScene->SetCamera(c->GetCam()); 
	}
	
	void Update(float deltaTime);
	
	Window* GetTargetWindow() 
	{
		assert(graphicsEngine); // TODO
		return nullptr;// graphicsEngine->GetTargetWindow();
	}
	
	inline gxeng::GraphicsEngine*	 GetGraphicsEngine() const { return graphicsEngine; }
	inline IPhysicsEngine*	 GetPhysicsEngine() const { return physicsEngine; }
	//inline INetworkEngine*	 GetNetworkEngine() const { return networkEngine; }
	//inline ISoundEngine*	 GetSoundEngine() const { return soundEngine; }

	inline std::vector<WorldComponent*> GetWorldComponents(eWorldComponentType type);
	inline std::vector<WorldComponent*> GetWorldComponents();

protected:
	IPhysicsEngine*	physicsEngine;
	//INetworkEngine*	networkEngine;
	//ISoundEngine*		soundEngine;

	// Scripts
	std::vector<Script*> scripts;

	// Actors
	std::vector<Actor*> actors;
	std::vector<Actor*> actorsToDestroy;

	// Prev and cur frame actors associated collision data
	//std::unordered_map<Actor*, core::Collision> curFrameActorCollideList;
	//std::unordered_map<Actor*, core::Collision> prevFrameActorCollideList;

	// Entity scripts
	std::vector<ActorScript*> entityScripts;

	// World components
	std::vector<WorldComponent*> worldComponents;

	// Tasks
	std::vector<Task> tasks;

	// Imported models
	//std::unordered_map<std::string, rImporter3DData*> importedModels;

	// Imported mono sounds
	//std::unordered_map<std::string, MonoSound> importedSounds;

	// Imported textures...
	std::unordered_map<std::string, gxeng::Texture2D*> importedTextures;

	// The default graphicsScene Core created for us to spawn graphics things into
	//graphics::IScene* defaultGraphicsScene;

	// The default soundScene Core created for us to spawn sound things into
	//sound::IScene* defaultSoundScene;

	// Error diffuse texture for failed texture loads
	gxeng::Texture2D* texError;


// Inline Engine specific things
	gxeng::GraphicsEngine* graphicsEngine;
	gxapi_dx12::GxapiManager* graphicsApiMgr;
	gxapi::IGraphicsApi* graphicsApi;

	GuiEngine* guiEngine;
	exc::Logger logger;
};

extern EngineCore Core;

std::vector<WorldComponent*> EngineCore::GetWorldComponents()
{
	return worldComponents;
}

std::vector<WorldComponent*> EngineCore::GetWorldComponents(eWorldComponentType type)
{
	std::vector<WorldComponent*> result;
	//result.clear();

	for (WorldComponent* c : GetWorldComponents())
	{
		if (c->GetType() == type)
		{
			result.push_back(c);
		}
	}

	return result;
}