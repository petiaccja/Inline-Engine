#include "EngineCore.hpp"
#include "PhysicsEngine/Common.hpp"
//#include "PlatformLibrary/Sys.hpp"
//#include "PlatformLibrary/File.hpp"
//#include "BaseLibrary/VisualCpuProfiler.hpp"
#include "Script.hpp"
#include "Actor.hpp"
#include <array>

EngineCore Core;

EngineCore::EngineCore()
:graphicsEngine(0), physicsEngine(0)/*, soundEngine(0), networkEngine(0)*/
{
	//curFrameActorCollideList.reserve(200);
	//prevFrameActorCollideList.reserve(200);
}

EngineCore::~EngineCore()
{

	for (auto& a : scripts)
		delete a;

	for (auto& a : worldComponents)
		delete a;

	for (auto& a : entityScripts)
		delete a;

	for (auto& a : actors)
		delete a;

	//for (auto& a : importedModels)
	//	delete a.second;
	//
	//for (auto& a : importedTextures)
	//	a.second->Release();

	//if (graphicsEngine)	graphicsEngine->Release();
	if (physicsEngine)	physicsEngine->Release();
	//if (networkEngine)	networkEngine->Release();
	//if (soundEngine)	soundEngine->Release();

	delete guiEngine;
	delete graphicsEngine;
	delete graphicsApi;
	delete graphicsApiMgr;
}

gxeng::GraphicsEngine* EngineCore::InitGraphicsEngine(int width, int height, HWND hwnd)
{
	if (graphicsEngine)
	{
		delete graphicsEngine;
		delete graphicsApi;
		delete graphicsApiMgr;
	}

	// Create Graphics Api
	GxapiManager* graphicsApiMgr = new GxapiManager();
	auto adapters = graphicsApiMgr->EnumerateAdapters();
	graphicsApi = graphicsApiMgr->CreateGraphicsApi(adapters[0].adapterId);

	// Create GraphicsEngine
	gxeng::GraphicsEngineDesc desc;
	desc.fullScreen = false;
	desc.graphicsApi = graphicsApi;
	desc.gxapiManager = graphicsApiMgr;
	desc.width = width;
	desc.height = height;
	desc.targetWindow = hwnd;
	desc.logger = &logger;

	graphicsEngine = new gxeng::GraphicsEngine(desc);

	return graphicsEngine;
}

GuiEngine* EngineCore::InitGuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow)
{
	if (guiEngine)
		delete guiEngine;

	guiEngine = new GuiEngine(graphicsEngine, targetWindow);

	return guiEngine;
}

IPhysicsEngine* EngineCore::InitPhysicsEngineBullet(const PhysicsEngineBulletDesc& d /*= PhysicsEngineBulletDesc()*/)
{
	if (physicsEngine)
		physicsEngine->Release();

	return physicsEngine = new PhysicsEngineBullet(d);
}

//INetworkEngine* EngineCore::InitNetworkEngineRakNet(const rNetworkEngine& d /*= rNetworkEngine()*/)
//{
//	if (networkEngine)
//		networkEngine->Release();
//
//	return networkEngine = new NetworkEngineRakNet(d);
//}
//
//ISoundEngine* EngineCore::InitSoundEngineSFML(const rSoundEngine& d /*= rSoundEngine()*/)
//{
//	if (soundEngine)
//		soundEngine->Release();
//
//	soundEngine = new SoundEngineSFML(d);
//
//	defaultSoundScene = soundEngine->CreateScene();
//
//	return soundEngine;
//}

bool EngineCore::TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceResult_out, const PhysicsTraceParams& params /*= PhysicsTraceParams()*/)
{
	physics::TraceResult result;
	if (physicsEngine->TraceClosestPoint(from, to, result, params))
	{
		traceResult_out.pos = result.pos;
		traceResult_out.normal = result.normal;

		RigidBodyEntity* entity = (RigidBodyEntity*)result.userPointer;

		if (entity)
			traceResult_out.actor = (Actor*)entity->GetUserPointer();
		else
			traceResult_out.actor = nullptr;

		return true;
	}
	return false;
}

//sound::IEmitter* EngineCore::CreateMonoSound(const std::string& filePath, float volumeNormedPercent /*= 1*/, bool bLoop /*= false*/)
//{
//	sound::IEmitter* soundEmitter;
//	sound::ISoundData* soundData;
//
//	auto it = importedSounds.find(filePath);
//	if (it != importedSounds.end())
//	{
//		soundData = it->second.soundData;
//		soundEmitter = it->second.soundEmitter;
//	}
//	else
//	{
//		soundData = soundEngine->CreateSoundData();
//		if (!soundData->Load((GetAssetsDir() + filePath).c_str(), sound::StoreMode::BUFFERED))
//		{
//			soundData->Release();
//			return false;
//		}
//
//		soundEmitter = defaultSoundScene->AddEmitter();
//		soundEmitter->SetSoundData(soundData);
//		soundEmitter->SetLooped(bLoop);
//
//		MonoSound d;
//		d.soundData = soundData;
//		d.soundEmitter = soundEmitter;
//		importedSounds[filePath] = d;
//	}
//
//	soundEmitter->SetVolume(volumeNormedPercent);
//	return soundEmitter;
//}

void EngineCore::RemoveActor(Actor* a)
{
	if (!a->IsKilled())
	{
		a->Kill();
		actorsToDestroy.push_back(a);
	}
}

void EngineCore::RemoveComponent(WorldComponent* c)
{
	// TODO
	assert(0);

	// TODO
	//if (c->IsMesh())
	//	defaultGraphicsScene->Remove(((MeshComponent*)c)->GetEntity());
	//else if (c->IsRigidBody())
	//	physicsEngine->RemoveEntity(((RigidBodyComponent*)c)->GetEntity());
	//else
	//	assert(0);
	//
	//auto it = std::find(worldComponents.begin(), worldComponents.end(), c);
	//if (it != worldComponents.end())
	//{
	//	delete *it;
	//	worldComponents.erase(it);
	//}
}

Actor* EngineCore::AddActor()
{
	Actor* actor = new Actor;
	actors.push_back(actor);
	return actor;
}

Actor* EngineCore::AddActor(const std::string& modelFilePath, float mass /*= 0*/)
{
	// Create Rigid body actor
	Actor* actor = AddActor_RigidBody(modelFilePath, mass);

	// Attach Graphics Mesh to it
	MeshComponent* meshComp = AddComponent_Mesh(modelFilePath);
	actor->GetRootComponent(0)->Attach(meshComp);

	return actor;
}

Actor* EngineCore::AddActor(WorldComponent* rootComp)
{
	Actor* actor = AddActor();
	actor->Attach(rootComp);

	return actor;
}

Actor* EngineCore::AddActor_Mesh(const std::string& modelFilePath)
{
	Actor* actor = AddActor(AddComponent_Mesh(modelFilePath));
	return actor;
}

Actor* EngineCore::AddActor_RigidBody(const std::string& modelFilePath, float mass)
{
	RigidBodyComponent* rigidComp = AddComponent_RigidBody(modelFilePath, mass);
	Actor* actor = AddActor(rigidComp);
	rigidComp->SetUserPointer(actor);

	return actor;
}

Actor* EngineCore::AddActor_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	RigidBodyComponent* rigidComp = AddComponent_RigidBodyCapsule(height, radius, mass);
	Actor* actor = AddActor(rigidComp);
	rigidComp->SetUserPointer(actor);

	return actor;
}

Actor* EngineCore::AddActor_Camera()
{
	return AddActor(AddComponent_Camera());
}

void EngineCore::AddTask(const std::function<void()>& callb, float timeToProceed)
{
	Task task;
	task.callb = callb;
	task.timeLeft = timeToProceed;
	tasks.push_back(task); // TODO slow
}

MeshComponent* EngineCore::AddComponent_Mesh(const std::string& modelAssetPath)
{
	assert(0); // TODO
	return nullptr;

	//rImporter3DData* modelDesc;
	//auto it = importedModels.find(modelAssetPath);
	//if (it != importedModels.end())
	//{
	//	modelDesc = it->second;
	//}
	//else // Not loaded, check bin format first
	//{
	//	std::string binPath = GetAssetsDir() + modelAssetPath.substr(0, modelAssetPath.rfind('.')) + ".exm"; // Excessive Mesh
	//
	//	if (File::IsExists(binPath))
	//	{
	//		modelDesc = new rImporter3DData();
	//		modelDesc->DeSerialize(binPath);
	//	}
	//	else
	//	{
	//		// Config for importing
	//		rImporter3DCfg cfg({ eImporter3DFlag::VERT_BUFF_INTERLEAVED,
	//			eImporter3DFlag::VERT_ATTR_POS,
	//			eImporter3DFlag::VERT_ATTR_NORM,
	//			eImporter3DFlag::VERT_ATTR_TAN,
	//			eImporter3DFlag::VERT_ATTR_TEX0,
	//			eImporter3DFlag::PIVOT_RECENTER});
	//
	//		modelDesc = new rImporter3DData();
	//		Importer3D::LoadModelFromFile(GetAssetsDir() + modelAssetPath, cfg, *modelDesc);
	//
	//		modelDesc->Serialize(binPath);
	//	}
	//
	//	importedModels[modelAssetPath] = modelDesc;
	//}
	//
	//graphics::IEntity* graphicsEntity;
	//
	//// We will feed meshes to that graphics entity
	//graphicsEntity = defaultGraphicsScene->AddEntity();
	//
	//// Material for entity
	//graphics::IMaterial* material = graphicsEngine->CreateMaterial();
	//graphicsEntity->SetMaterial(material);
	//
	//// For each mesh imported, Create graphics mesh
	//for (auto& importedMesh : modelDesc->meshes)
	//{
	//	graphics::IMesh* graphicsMesh = graphicsEngine->CreateMesh();
	//	graphicsEntity->SetMesh(graphicsMesh);
	//
	//	// Materials
	//	for (auto& importedMaterial : importedMesh->materials)
	//	{
	//		auto& subMat = material->AddSubMaterial();
	//		subMat.base = mm::vec4(1, 1, 1, 1);
	//		subMat.t_diffuse = texError; // Default is error texture !!
	//
	//		if (importedMaterial.relTexPathDiffuse != "")
	//		{
	//			// TODO:
	//			// turn .bmp references into .jpg (UGLY TMP)
	//			// Todo, ".bmp", ".BMP"...
	//			std::string relPath;
	//			if (importedMaterial.relTexPathDiffuse.rfind(".bmp") != std::string::npos || importedMaterial.relTexPathDiffuse.rfind(".BMP") != std::string::npos)
	//			{
	//				auto idx = importedMaterial.relTexPathDiffuse.rfind('.');
	//				auto jpgExtension = importedMaterial.relTexPathDiffuse.substr(0, idx + 1) + "jpg";
	//				relPath = jpgExtension;
	//			}
	//			else
	//			{
	//				relPath = importedMaterial.relTexPathDiffuse;
	//			}
	//
	//			graphics::ITexture* texDiffuse;
	//			auto it = importedTextures.find(relPath);
	//			if (it != importedTextures.end())
	//			{
	//				texDiffuse = it->second;
	//			}
	//			else
	//			{
	//				texDiffuse = graphicsEngine->CreateTexture();
	//				if (texDiffuse->Load(GetAssetsDir() + relPath))
	//					importedTextures[relPath] = texDiffuse;
	//				else
	//					texDiffuse = texError;
	//			}
	//			subMat.t_diffuse = texDiffuse;
	//		}
	//	}
	//
	//	// Material groups (face assignment)
	//	std::vector<graphics::IMesh::MaterialGroup> matIDs;
	//	matIDs.resize(importedMesh->materials.size());
	//	for (uint32_t i = 0; i < matIDs.size(); i++)
	//	{
	//		matIDs[i].beginFace = importedMesh->materials[i].faceStartIdx;
	//		matIDs[i].endFace = importedMesh->materials[i].faceEndIdx;
	//		matIDs[i].id = i;
	//	}
	//
	//	graphics::IMesh::MeshData meshData;
	//	meshData.index_data = (uint32_t*)importedMesh->indices;
	//	meshData.index_num = importedMesh->nIndices;
	//	meshData.mat_ids = matIDs.data();
	//	meshData.mat_ids_num = (uint32_t)matIDs.size();
	//	meshData.vertex_bytes = importedMesh->nVertices * importedMesh->vertexSize;
	//	meshData.vertex_data = importedMesh->vertexBuffers[0];
	//
	//	graphics::IMesh::ElementDesc elements[] = {
	//		graphics::IMesh::POSITION, 3,
	//		graphics::IMesh::NORMAL, 3,
	//		graphics::IMesh::TANGENT, 3,
	//		graphics::IMesh::TEX0, 2,
	//	};
	//	meshData.vertex_elements = elements;
	//	meshData.vertex_elements_num = sizeof(elements) / sizeof(elements[0]);
	//
	//	// Feed data to mesh
	//	graphicsMesh->Update(meshData);
	//}
	//
	//auto c = new MeshComponent(graphicsEntity);
	//worldComponents.push_back(c);
	//return c;
}

RigidBodyComponent* EngineCore::AddComponent_RigidBody(const std::string& modelAssetPath, float mass)
{
	assert(0); // TODO
	return nullptr;

	// Check if model already loaded somehow
	//rImporter3DData* modelDesc;
	//auto it = importedModels.find(modelAssetPath);
	//if (it != importedModels.end())
	//{
	//	modelDesc = it->second;
	//}
	//else // Not loaded, check bin format first
	//{
	//	std::string binPath = GetAssetsDir() + modelAssetPath.substr(0, modelAssetPath.rfind('.')) + ".exm"; // Excessive Mesh
	//
	//	if (File::IsExists(binPath))
	//	{
	//		modelDesc = new rImporter3DData();
	//		modelDesc->DeSerialize(binPath);
	//	}
	//	else
	//	{
	//		// Config for importing
	//		rImporter3DCfg cfg({ eImporter3DFlag::VERT_BUFF_INTERLEAVED,
	//			eImporter3DFlag::VERT_ATTR_POS,
	//			eImporter3DFlag::VERT_ATTR_NORM,
	//			eImporter3DFlag::VERT_ATTR_TAN,
	//			eImporter3DFlag::VERT_ATTR_TEX0,
	//			eImporter3DFlag::PIVOT_RECENTER });
	//
	//		modelDesc = new rImporter3DData();
	//		Importer3D::LoadModelFromFile(GetAssetsDir() + modelAssetPath, cfg, *modelDesc);
	//
	//		modelDesc->Serialize(binPath);
	//	}
	//
	//	importedModels[modelAssetPath] = modelDesc;
	//}
	//
	//physics::IRigidBodyEntity* rigidEntity = nullptr;
	//
	//auto mesh = modelDesc->meshes[0];
	//
	//Vec3* vertices;
	////if (cfg.isContain(eImporter3DFlag::VERT_BUFF_INTERLEAVED)) // Interleaved buffer? Okay gather positions from vertices stepping with vertex stride
	//{
	//	vertices = new Vec3[mesh->nVertices];
	//	for (uint32_t i = 0; i < mesh->nVertices; i++)
	//	{
	//		vertices[i] = *(Vec3*)((uint8_t*)mesh->vertexBuffers[0] + i * mesh->vertexSize);
	//	}
	//
	//}
	//
	//if (mass == 0)
	//	rigidEntity = physicsEngine->AddEntityRigidStatic(vertices, mesh->nVertices, mesh->indices, mesh->indexSize, mesh->nIndices);
	//else
	//	rigidEntity = physicsEngine->AddEntityRigidDynamic(vertices, mesh->nVertices, mass);
	//
	////loadedPhysicalVertexPositions = vertices;
	////delete vertices;
	////vertices = nullptr; // Important
	//
	//auto c = new RigidBodyComponent(rigidEntity);
	//worldComponents.push_back(c);
	//return c;
}

RigidBodyComponent* EngineCore::AddComponent_RigidBodyCapsule(float height, float radius, float mass /* = 0*/)
{
	auto capsuleEntity = physicsEngine->AddEntityRigidCapsule(height, radius, mass);

	auto c = new RigidBodyComponent(capsuleEntity);
	worldComponents.push_back(c);
	return c;
}

PerspectiveCameraComponent* EngineCore::AddComponent_Camera()
{
	auto c = new PerspectiveCameraComponent(graphicsEngine->CreatePerspectiveCamera(""));
	worldComponents.push_back(c);
	return c;
}

Transform3DComponent* EngineCore::AddComponent_Transform3D()
{
	auto c = new Transform3DComponent();
	worldComponents.push_back(c);
	return c;
}

void EngineCore::SetLayerCollision(size_t ID0, size_t ID1, bool bEnableCollision)
{
	assert(physicsEngine);
	physicsEngine->SetLayerCollision(ID0, ID1, bEnableCollision);
}

void EngineCore::Update(float deltaTime)
{
	graphicsEngine->Update(deltaTime);
	guiEngine->Update(deltaTime);

//	{
//		PROFILE_SCOPE("Game Logic");
//
//		// Collision, enter, exit calls
//		if (physicsEngine)
//		{
//			PROFILE_SCOPE("Core & onCollision(Enter, Exit, ...)");
//		
//			const std::vector<physics::Collision>& collisionList = physicsEngine->GetCollisionList();
//		
//			curFrameActorCollideList.clear();
//		
//			for (auto& collision : collisionList)
//			{
//				// Fill up Collision, based on rPhysicsCollision
//				Collision colData;
//		
//				// A
//				if (collision.rigidBodyA)
//				{
//					colData.softBodyA = nullptr;
//					colData.actorA = (Actor*)collision.rigidBodyA->GetUserPointer();
//		
//					if (colData.actorA)
//					{
//						// Search for component associated with RigidBodyEntity
//						// Todo: optimize
//						std::vector<RigidBodyComponent*> rigidBodyComps = colData.actorA->GetComponents<RigidBodyComponent>();
//						for (auto& a : rigidBodyComps)
//						{
//							if (a->GetEntity() == collision.rigidBodyA)
//							{
//								colData.rigidBodyA = a;
//								break;
//							}
//						}
//					}
//					else if (colData.actorA->IsKilled())
//					{
//						colData.actorA = nullptr;
//					}
//				}
//				else if (collision.softBodyA)
//				{
//					colData.rigidBodyA = nullptr;
//					colData.actorA = (Actor*)collision.softBodyA->GetUserPointer();
//		
//					if (colData.actorA)
//					{
//						// Search for component associated with RigidBodyEntity
//						// Todo: optimize
//						std::vector<SoftBodyComponent*> softBodyComps = colData.actorA->GetComponents<SoftBodyComponent>();
//						for (auto& a : softBodyComps)
//						{
//							if (a->GetEntity() == collision.softBodyA)
//							{
//								colData.softBodyA = a;
//								break;
//							}
//						}
//					}
//					else if (colData.actorA->IsKilled())
//					{
//						colData.actorA = nullptr;
//					}
//				}
//				else
//				{
//					colData.actorA = nullptr;
//				}
//		
//				// B
//				if (collision.rigidBodyB)
//				{
//					colData.softBodyB = nullptr;
//					colData.actorB = (Actor*)collision.rigidBodyB->GetUserPointer();
//		
//					if (colData.actorB)
//					{
//						// Search for component associated with RigidBodyEntity
//						// Todo: optimize
//						std::vector<RigidBodyComponent*> rigidBodyComps = colData.actorB->GetComponents<RigidBodyComponent>();
//						for (auto& a : rigidBodyComps)
//						{
//							if (a->GetEntity() == collision.rigidBodyB)
//							{
//								colData.rigidBodyB = a;
//								break;
//							}
//						}
//					}
//					else if (colData.actorB->IsKilled())
//					{
//						colData.actorB = nullptr;
//					}
//				}
//				else if (collision.softBodyB)
//				{
//					colData.rigidBodyB = nullptr;
//					colData.actorB = (Actor*)collision.softBodyB->GetUserPointer();
//		
//					if (colData.actorB)
//					{
//						// Search for component associated with RigidBodyEntity
//						// Todo: optimize
//						std::vector<SoftBodyComponent*> softBodyComps = colData.actorB->GetComponents<SoftBodyComponent>();
//						for (auto& a : softBodyComps)
//						{
//							if (a->GetEntity() == collision.softBodyB)
//							{
//								colData.softBodyB = a;
//								break;
//							}
//						}
//					}
//					else if (colData.actorB->IsKilled())
//					{
//						colData.actorB = nullptr;
//					}
//				}
//				else
//				{
//					colData.actorB = nullptr;
//				}
//		
//				if ((size_t)colData.actorA | (size_t)colData.actorB)
//					colData.contacts = collision.contacts;
//				else
//					continue;
//		
//				auto CheckDispatchActoCollision = [&](Actor* a)
//				{
//					curFrameActorCollideList[a] = colData;
//		
//					// YES cur frame data (OnCollision)
//					if (a->GetOnCollision())
//						a->GetOnCollision()(colData);
//		
//					// If previous frame collided actor not found in current list, then Call OnCollisionExit
//					bool bActorFound = false;
//					for (auto& aPrev : prevFrameActorCollideList)
//					{
//						if (aPrev.first != a || aPrev.first->IsKilled())
//							continue;
//		
//						bActorFound = true;
//		
//						const auto& collision = aPrev.second;
//						if (collision.actorA == aPrev.first && collision.actorA->GetOnCollisionExit() != nullptr)
//							collision.actorA->GetOnCollisionExit()(collision);
//						else if (collision.actorB == aPrev.first && collision.actorB->GetOnCollisionExit() != nullptr)
//							collision.actorB->GetOnCollisionExit()(collision);
//					}
//		
//					// NO prev frame data, YES cur frame data for actor (OnCollisionEnter)
//					if (!bActorFound && a->GetOnCollisionEnter())
//						a->GetOnCollisionEnter()(colData);
//				};
//		
//				if (colData.actorA)
//					CheckDispatchActoCollision(colData.actorA);
//		
//				if (colData.actorB)
//					CheckDispatchActoCollision(colData.actorB);
//			}
//		
//			prevFrameActorCollideList = curFrameActorCollideList;
//		}
//
//		// Scripts
//		{
//			PROFILE_SCOPE("Scripts");
//			for (auto& s : scripts)
//				s->Update(deltaTime);
//		}
//
//
//		//Entity Scripts
//		{
//			PROFILE_SCOPE("Entity Scripts");
//			for (auto& s : entityScripts)
//				s->Update(deltaTime);
//		}
//
//
//		// ActorLambda onUpdate
//		{
//			PROFILE_SCOPE("ActorLambda onUpdate");
//			for (auto& a : actors)
//				if (!a->IsKilled())
//					a->Update(deltaTime);
//		}
//
//		// TODO optimize
//		// Process Tasks if time passed, and remove after dispatch
//		{
//			PROFILE_SCOPE("AddTask Dispatch");
//
//			static std::vector<size_t> indicesToDelete;
//			indicesToDelete.clear();
//
//			size_t oldSize = tasks.size(); // Cuz you can AddTask in AddTask ^^, tasks.size() can change in loop body
//			for (size_t i = 0; i < oldSize; i++)
//			{
//				tasks[i].timeLeft -= deltaTime;
//
//				if (tasks[i].timeLeft <= 0)
//				{
//					tasks[i].callb();
//					indicesToDelete.push_back(i);
//				}
//			}
//
//			// Remove dispatched taks
//			for (i32 i = 0; i < (i32)indicesToDelete.size(); ++i)
//				tasks.erase(tasks.begin() + indicesToDelete[i] - i);
//		}
//	}
//
//	// Destroy actors queued for destroying
//	{
//		PROFILE_SCOPE("Destroying Actors");
//		for (auto& a : actorsToDestroy)
//		{
//			// Remove from actors
//			auto it = std::find(actors.begin(), actors.end(), a);
//			if (it != actors.end())
//				actors.erase(it);
//
//			// Remove from prevFrameCollision datas (Actor*)
//			auto it2 = prevFrameActorCollideList.begin();
//			while (it2 != prevFrameActorCollideList.end())
//			{
//				if (it2->first == a)
//				{
//					it2 = prevFrameActorCollideList.erase(it2);
//					continue;
//				}
//			
//				it2++;
//			}
//
//			for (auto& comp : a->GetComponents())
//				Remove(comp);
//
//			delete a;
//		}
//		actorsToDestroy.clear();
//	}
//
//
//	//(RigidBody) Entity -> Component transform update
//	{
//		PROFILE_SCOPE("(RigidBody) Entity -> Component transform update");
//	
//		for (WorldComponent* c : GetWorldComponents(RIGID_BODY))
//		{
//			physics::IRigidBodyEntity* entity = ((RigidBodyComponent*)c)->GetEntity();
//			entity->SetPos(c->GetPos());
//			entity->SetRot(c->GetRot());
//			entity->SetScale(c->GetScale());
//		}
//	}
//	
//	// (Mesh) Entity -> Component transform update
//	{
//		PROFILE_SCOPE("(Mesh) Entity -> Component transform update");
//	
//		for (WorldComponent* c : GetWorldComponents(MESH))
//		{
//			graphics::IEntity* entity = ((MeshComponent*)c)->GetEntity();
//			entity->SetPos(c->GetPos());
//			entity->SetRot(c->GetRot());
//			entity->SetScale(c->GetScale());
//			entity->SetSkew(c->GetSkew());
//		}
//	}
//	
//	// (Camera) Entity -> Component transform update
//	{
//		PROFILE_SCOPE("(Camera) Entity -> Component transform update");
//	
//		for (WorldComponent* c : GetWorldComponents(CAMERA))
//		{
//			ICamera* entity = ((PerspectiveCameraComponent*)c)->GetCam();
//			entity->SetPos(c->GetPos());
//			entity->SetRot(c->GetRot());
//		}
//	}
//
//
//	// Update physics
//	if (physicsEngine)
//	{
//		PROFILE_SCOPE("Physics");
//		physicsEngine->Update(deltaTime);
//	}
//
//	//Update rigid body components, from rigid body entity
//	{
//		PROFILE_SCOPE("Update rigid body components, from rigid body entity");
//	
//		for (WorldComponent* c : GetWorldComponents(RIGID_BODY))
//		{
//			physics::IRigidBodyEntity* entity = ((RigidBodyComponent*)c)->GetEntity();
//			c->SetPos(entity->GetPos());
//			c->SetRot(entity->GetRot());
//			c->SetScale(entity->GetScale());
//		}
//	}
//	
//
//	// Update graphics
//	if (graphicsEngine)
//	{
//		PROFILE_SCOPE("Graphics");
//
//#ifdef PROFILE_ENGINE
//		graphicsEngine->GetGapi()->ResetStatesToDefault(); // Jesus the profiler also uses OpenGL temporarily, and mess up the binds etc...
//#endif
//		graphicsEngine->Update(deltaTime);
//	}
//
//	if(guiEngine)
//	{
//		guiEngine->Update(deltaTime);
//	}
//
//	// Update sound
//	if (soundEngine)
//	{
//		PROFILE_SCOPE("Sound");
//		soundEngine->Update(deltaTime);
//	}
//
//	// Update network
//	if (networkEngine)
//	{
//		PROFILE_SCOPE("Network");
//		networkEngine->Update(deltaTime);
//	}
//
//	// Profiling engine
//#ifdef PROFILE_ENGINE
//	VisualCpuProfiler::UpdateAndPresent();
//#endif
}