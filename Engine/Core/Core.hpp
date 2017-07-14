#pragma once
// The purpose of this class is to take the most minimal input from the user to (startup, use) the engine
#include <GuiEngine/GuiEngine.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine/Definitions.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <PhysicsEngine/IPhysicsEngine.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <PhysicsEngine/Bullet/PhysicsEngineBullet.hpp>

#include "InputCore.hpp"
#include "ActorScript.hpp"
#include "MeshPart.hpp"
#include "RigidBodyPart.hpp"
#include "PerspCameraPart.hpp"
#include "Common.hpp"
#include "Scene.hpp"

#include <unordered_map>
#include <vector>
#include <functional>

namespace inl::core {

class SceneScript;

using namespace inl;
using namespace inl::gui;
using namespace inl::physics::bullet;
using namespace inl::gxapi;
using namespace inl::gxapi_dx12;

class Core
{
public:
	Core();
	~Core();

	gxeng::GraphicsEngine*	InitGraphicsEngine(int width, int height, HWND hwnd);
	IPhysicsEngine*			InitPhysicsEngineBullet();
	GuiEngine*				InitGuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow);

	//bool TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, core::PhysicsTraceResult& traceResult_out, const core::PhysicsTraceParams& params = core::PhysicsTraceParams());

	// Todo
	//bool TraceClosestPoint_Graphics(const Vec3& from, const Vec3& to, TraceResult& traceInfo_out);
	
	//sound::IEmitter* CreateMonoSound(const std::string& filePath, float volumeNormedPercent = 1, bool bLoop = false);
	
	//Actor* AddActor();
	//Actor* AddActor(const std::string& modelFilePath, float mass = 0);
	//Actor* AddActor(Part* rootComp);
	//Actor* AddActor_Mesh(const std::string& modelFilePath);
	//Actor* AddActor_RigidBody(const std::string& modelFilePath, float mass);
	//Actor* AddActor_RigidBodyCapsule(float height, float radius, float mass = 0);
	//Actor* AddActor_Camera();
	//
	//void RemoveActor(Actor* a);
	//void RemovePart(Part* c);

	void AddTask(const std::function<void()>& callb, float timeToProceed);
	
	template<class ScriptClass>
	SceneScript* AddScript()
	{
		ScriptClass* p = new ScriptClass();
		scripts.push_back(p);
		return p;
	}
	
	template<class ActorScriptClass>
	ActorScript* AddActorScript()
	{
		ActorScriptClass* p = new ActorScriptClass();
		p->SetActor(Core.AddEntity());
	
		actorScripts.push_back(p);
		return p;
	}
	
	void Update(float deltaTime);
	
	Window* GetTargetWindow() 
	{
		assert(graphicsEngine); // TODO
		return nullptr;// graphicsEngine->GetTargetWindow();
	}
	
	gxeng::GraphicsEngine*	 GetGraphicsEngine() const { return graphicsEngine; }
	IPhysicsEngine*	 GetPhysicsEngine() const { return physicsEngine; }
	//inline INetworkEngine*	 GetNetworkEngine() const { return networkEngine; }
	//inline ISoundEngine*	 GetSoundEngine() const { return soundEngine; }

	std::vector<Part*> GetParts(ePartType type);
	std::vector<Part*> GetParts();

protected:
	IPhysicsEngine*	physicsEngine;
	//INetworkEngine*	networkEngine;
	//ISoundEngine*		soundEngine;

	// Scripts
	std::vector<SceneScript*> scripts;

	// Actors
	std::vector<Actor*> actors;
	std::vector<Actor*> actorsToDestroy;

	// Prev and cur frame actors associated collision data
	//std::unordered_map<Actor*, core::Collision> curFrameActorCollideList;
	//std::unordered_map<Actor*, core::Collision> prevFrameActorCollideList;

	// Actor scripts
	std::vector<ActorScript*> actorScripts;

	// World components
	std::vector<Part*> parts;
	std::vector<Part*> partsToDestroy;

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

extern Core gCore;
extern Scene gScene;
extern InputCore gInput;

} // namespace inl::core