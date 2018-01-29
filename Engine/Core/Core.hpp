#pragma once
// The purpose of this class is to take the most minimal input from the user to (startup, use) the engine
#include <GuiEngine/GuiEngine.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine/Definitions.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
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
using namespace inl::gxapi;
using namespace inl::gxapi_dx12;
using namespace inl::physics;

class Core
{
public:
	Core();
	~Core();

	gxeng::GraphicsEngine*					InitGraphicsEngine(int width, int height, HWND hwnd);
	physics::bullet::PhysicsEngineBullet*	InitPhysicsEngineBullet();
	GuiEngine*								InitGuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow);

	void AddTask(const std::function<void()>& function, float timeToProceed);
	
	Scene* CreateScene();

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
	physics::bullet::PhysicsEngineBullet*	 GetPhysicsEngine() const { return physicsEngine; }
	//inline INetworkEngine*	 GetNetworkEngine() const { return networkEngine; }
	//inline ISoundEngine*	 GetSoundEngine() const { return soundEngine; }

	//std::vector<Part*> GetParts(ePartType type);
	//std::vector<Part*> GetParts();

protected:
	// Physics engine
	physics::bullet::PhysicsEngineBullet*	physicsEngine;

	// Graphics engine
	gxeng::GraphicsEngine* graphicsEngine;

	// guiEngine
	GuiEngine* guiEngine;

	std::vector<Scene*> scenes;

	// Scripts operating on scenes
	std::vector<SceneScript*> sceneScripts;

	// Scripts operating on actors
	std::vector<ActorScript*> actorScripts;

	// Actors
	std::vector<Actor*> actors;
	//std::vector<Actor*> actorsToDestroy;

	std::vector<Part*> parts;
	//std::vector<Part*> partsToDestroy;

	// Prev and cur frame actors associated collision data
	//std::unordered_map<Actor*, core::Collision> curFrameActorCollideList;
	//std::unordered_map<Actor*, core::Collision> prevFrameActorCollideList;

	

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

	// Texture applied to meshes which have no associated texture
	//gxeng::Texture2D* errorTexture;

	gxapi_dx12::GxapiManager* graphicsApiMgr;
	gxapi::IGraphicsApi* graphicsApi;
	inl::Logger logger;
};

} // namespace inl::core