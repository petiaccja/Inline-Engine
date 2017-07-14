#include "Core.hpp"
#include "Actor.hpp"

#include <PhysicsEngine/Common.hpp>
#include <array>
#include <BaseLibrary\VisualCpuProfiler.h>

namespace inl::core {

Core gCore;
Scene gScene;
InputCore gInput;

Core::Core()
:graphicsEngine(0), physicsEngine(0)/*, soundEngine(0), networkEngine(0)*/
{

}

Core::~Core()
{

	for (auto& a : scripts)
		delete a;

	for (auto& a : parts)
		delete a;

	for (auto& a : actorScripts)
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

gxeng::GraphicsEngine* Core::InitGraphicsEngine(int width, int height, HWND hwnd)
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

GuiEngine* Core::InitGuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow)
{
	if (guiEngine)
		delete guiEngine;

	guiEngine = new GuiEngine(graphicsEngine, targetWindow);

	return guiEngine;
}

IPhysicsEngine* Core::InitPhysicsEngineBullet()
{
	if (physicsEngine)
		physicsEngine->Release();

	bullet::PhysicsEngineBullet* engine = new bullet::PhysicsEngineBullet();
	return engine;
}

//INetworkEngine* Core::InitNetworkEngineRakNet(const rNetworkEngine& d /*= rNetworkEngine()*/)
//{
//	if (networkEngine)
//		networkEngine->Release();
//
//	return networkEngine = new NetworkEngineRakNet(d);
//}
//
//ISoundEngine* Core::InitSoundEngineSFML(const rSoundEngine& d /*= rSoundEngine()*/)
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

//bool Core::TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceResult_out, const PhysicsTraceParams& params /*= PhysicsTraceParams()*/)
//{
//	physics::TraceResult result;
//	if (physicsEngine->TraceClosestPoint(from, to, result, params))
//	{
//		traceResult_out.pos = result.pos;
//		traceResult_out.normal = result.normal;
//
//		RigidBodyEntity* entity = (RigidBodyEntity*)result.userPointer;
//
//		if (entity)
//			traceResult_out.actor = (Actor*)entity->GetUserPointer();
//		else
//			traceResult_out.actor = nullptr;
//
//		return true;
//	}
//	return false;
//}

//sound::IEmitter* Core::CreateMonoSound(const std::string& filePath, float volumeNormedPercent /*= 1*/, bool bLoop /*= false*/)
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

//void Core::RemoveActor(Actor* a)
//{
//	if (!a->IsKilled())
//	{
//		a->Kill();
//		actorsToDestroy.push_back(a);
//	}
//}
//
//void Core::RemovePart(Part* p)
//{
//	if (!p->IsKilled())
//	{
//		p->Kill();
//		partsToDestroy.push_back(p);
//	}
//
//	// TODO
//	assert(0);
//
//	// TODO
//	//if (c->IsMesh())
//	//	defaultGraphicsScene->Remove(((MeshPart*)c)->GetEntity());
//	//else if (c->IsRigidBody())
//	//	physicsEngine->RemoveEntity(((RigidBodyPart*)c)->GetEntity());
//	//else
//	//	assert(0);
//	//
//	//auto it = std::find(parts.begin(), parts.end(), c);
//	//if (it != parts.end())
//	//{
//	//	delete *it;
//	//	parts.erase(it);
//	//}
//}

//Actor* Core::AddActor()
//{
//	Actor* actor = new Actor;
//	actors.push_back(actor);
//	return actor;
//}
//
//Actor* Core::AddActor(const std::string& modelFilePath, float mass /*= 0*/)
//{
//	// Create Rigid body actor
//	Actor* actor = AddActor_RigidBody(modelFilePath, mass);
//
//	// Attach Graphics Mesh to it
//	MeshPart* meshComp = AddPart_Mesh(modelFilePath);
//	actor->GetRootPart(0)->Attach(meshComp);
//
//	return actor;
//}
//
//Actor* Core::AddActor(Part* rootComp)
//{
//	Actor* actor = AddActor();
//	actor->Attach(rootComp);
//
//	return actor;
//}
//
//Actor* Core::AddActor_Mesh(const std::string& modelFilePath)
//{
//	Actor* actor = AddActor(AddPart_Mesh(modelFilePath));
//	return actor;
//}
//
//Actor* Core::AddActor_RigidBody(const std::string& modelFilePath, float mass)
//{
//	RigidBodyPart* rigidComp = AddPart_RigidBody(modelFilePath, mass);
//	Actor* actor = AddActor(rigidComp);
//	rigidComp->SetUserPointer(actor);
//
//	return actor;
//}
//
//Actor* Core::AddActor_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
//{
//	RigidBodyPart* rigidComp = AddPart_RigidBodyCapsule(height, radius, mass);
//	Actor* actor = AddActor(rigidComp);
//	rigidComp->SetUserPointer(actor);
//
//	return actor;
//}
//
//Actor* Core::AddActor_Camera()
//{
//	return AddActor(AddPart_Camera());
//}
//
//void Core::AddTask(const std::function<void()>& callb, float timeToProceed)
//{
//	Task task;
//	task.callb = callb;
//	task.timeLeft = timeToProceed;
//	tasks.push_back(task); // TODO slow
//}
//
//MeshPart* Core::AddPart_Mesh(const std::string& modelAssetPath)
//{
//	assert(0); // TODO
//	return nullptr;
//
//	//rImporter3DData* modelDesc;
//	//auto it = importedModels.find(modelAssetPath);
//	//if (it != importedModels.end())
//	//{
//	//	modelDesc = it->second;
//	//}
//	//else // Not loaded, check bin format first
//	//{
//	//	std::string binPath = GetAssetsDir() + modelAssetPath.substr(0, modelAssetPath.rfind('.')) + ".exm"; // Excessive Mesh
//	//
//	//	if (File::IsExists(binPath))
//	//	{
//	//		modelDesc = new rImporter3DData();
//	//		modelDesc->DeSerialize(binPath);
//	//	}
//	//	else
//	//	{
//	//		// Config for importing
//	//		rImporter3DCfg cfg({ eImporter3DFlag::VERT_BUFF_INTERLEAVED,
//	//			eImporter3DFlag::VERT_ATTR_POS,
//	//			eImporter3DFlag::VERT_ATTR_NORM,
//	//			eImporter3DFlag::VERT_ATTR_TAN,
//	//			eImporter3DFlag::VERT_ATTR_TEX0,
//	//			eImporter3DFlag::PIVOT_RECENTER});
//	//
//	//		modelDesc = new rImporter3DData();
//	//		Importer3D::LoadModelFromFile(GetAssetsDir() + modelAssetPath, cfg, *modelDesc);
//	//
//	//		modelDesc->Serialize(binPath);
//	//	}
//	//
//	//	importedModels[modelAssetPath] = modelDesc;
//	//}
//	//
//	//graphics::IEntity* graphicsEntity;
//	//
//	//// We will feed meshes to that graphics entity
//	//graphicsEntity = defaultGraphicsScene->AddEntity();
//	//
//	//// Material for entity
//	//graphics::IMaterial* material = graphicsEngine->CreateMaterial();
//	//graphicsEntity->SetMaterial(material);
//	//
//	//// For each mesh imported, Create graphics mesh
//	//for (auto& importedMesh : modelDesc->meshes)
//	//{
//	//	graphics::IMesh* graphicsMesh = graphicsEngine->CreateMesh();
//	//	graphicsEntity->SetMesh(graphicsMesh);
//	//
//	//	// Materials
//	//	for (auto& importedMaterial : importedMesh->materials)
//	//	{
//	//		auto& subMat = material->AddSubMaterial();
//	//		subMat.base = mm::vec4(1, 1, 1, 1);
//	//		subMat.t_diffuse = texError; // Default is error texture !!
//	//
//	//		if (importedMaterial.relTexPathDiffuse != "")
//	//		{
//	//			// TODO:
//	//			// turn .bmp references into .jpg (UGLY TMP)
//	//			// Todo, ".bmp", ".BMP"...
//	//			std::string relPath;
//	//			if (importedMaterial.relTexPathDiffuse.rfind(".bmp") != std::string::npos || importedMaterial.relTexPathDiffuse.rfind(".BMP") != std::string::npos)
//	//			{
//	//				auto idx = importedMaterial.relTexPathDiffuse.rfind('.');
//	//				auto jpgExtension = importedMaterial.relTexPathDiffuse.substr(0, idx + 1) + "jpg";
//	//				relPath = jpgExtension;
//	//			}
//	//			else
//	//			{
//	//				relPath = importedMaterial.relTexPathDiffuse;
//	//			}
//	//
//	//			graphics::ITexture* texDiffuse;
//	//			auto it = importedTextures.find(relPath);
//	//			if (it != importedTextures.end())
//	//			{
//	//				texDiffuse = it->second;
//	//			}
//	//			else
//	//			{
//	//				texDiffuse = graphicsEngine->CreateTexture();
//	//				if (texDiffuse->Load(GetAssetsDir() + relPath))
//	//					importedTextures[relPath] = texDiffuse;
//	//				else
//	//					texDiffuse = texError;
//	//			}
//	//			subMat.t_diffuse = texDiffuse;
//	//		}
//	//	}
//	//
//	//	// Material groups (face assignment)
//	//	std::vector<graphics::IMesh::MaterialGroup> matIDs;
//	//	matIDs.resize(importedMesh->materials.size());
//	//	for (uint32_t i = 0; i < matIDs.size(); i++)
//	//	{
//	//		matIDs[i].beginFace = importedMesh->materials[i].faceStartIdx;
//	//		matIDs[i].endFace = importedMesh->materials[i].faceEndIdx;
//	//		matIDs[i].id = i;
//	//	}
//	//
//	//	graphics::IMesh::MeshData meshData;
//	//	meshData.index_data = (uint32_t*)importedMesh->indices;
//	//	meshData.index_num = importedMesh->nIndices;
//	//	meshData.mat_ids = matIDs.data();
//	//	meshData.mat_ids_num = (uint32_t)matIDs.size();
//	//	meshData.vertex_bytes = importedMesh->nVertices * importedMesh->vertexSize;
//	//	meshData.vertex_data = importedMesh->vertexBuffers[0];
//	//
//	//	graphics::IMesh::ElementDesc elements[] = {
//	//		graphics::IMesh::POSITION, 3,
//	//		graphics::IMesh::NORMAL, 3,
//	//		graphics::IMesh::TANGENT, 3,
//	//		graphics::IMesh::TEX0, 2,
//	//	};
//	//	meshData.vertex_elements = elements;
//	//	meshData.vertex_elements_num = sizeof(elements) / sizeof(elements[0]);
//	//
//	//	// Feed data to mesh
//	//	graphicsMesh->Update(meshData);
//	//}
//	//
//	//auto c = new MeshPart(graphicsEntity);
//	//parts.push_back(c);
//	//return c;
//}
//
//RigidBodyPart* Core::AddPart_RigidBody(const std::string& modelAssetPath, float mass)
//{
//	assert(0); // TODO
//	return nullptr;
//
//	// Check if model already loaded somehow
//	//rImporter3DData* modelDesc;
//	//auto it = importedModels.find(modelAssetPath);
//	//if (it != importedModels.end())
//	//{
//	//	modelDesc = it->second;
//	//}
//	//else // Not loaded, check bin format first
//	//{
//	//	std::string binPath = GetAssetsDir() + modelAssetPath.substr(0, modelAssetPath.rfind('.')) + ".exm"; // Excessive Mesh
//	//
//	//	if (File::IsExists(binPath))
//	//	{
//	//		modelDesc = new rImporter3DData();
//	//		modelDesc->DeSerialize(binPath);
//	//	}
//	//	else
//	//	{
//	//		// Config for importing
//	//		rImporter3DCfg cfg({ eImporter3DFlag::VERT_BUFF_INTERLEAVED,
//	//			eImporter3DFlag::VERT_ATTR_POS,
//	//			eImporter3DFlag::VERT_ATTR_NORM,
//	//			eImporter3DFlag::VERT_ATTR_TAN,
//	//			eImporter3DFlag::VERT_ATTR_TEX0,
//	//			eImporter3DFlag::PIVOT_RECENTER });
//	//
//	//		modelDesc = new rImporter3DData();
//	//		Importer3D::LoadModelFromFile(GetAssetsDir() + modelAssetPath, cfg, *modelDesc);
//	//
//	//		modelDesc->Serialize(binPath);
//	//	}
//	//
//	//	importedModels[modelAssetPath] = modelDesc;
//	//}
//	//
//	//physics::IRigidBodyEntity* rigidEntity = nullptr;
//	//
//	//auto mesh = modelDesc->meshes[0];
//	//
//	//Vec3* vertices;
//	////if (cfg.isContain(eImporter3DFlag::VERT_BUFF_INTERLEAVED)) // Interleaved buffer? Okay gather positions from vertices stepping with vertex stride
//	//{
//	//	vertices = new Vec3[mesh->nVertices];
//	//	for (uint32_t i = 0; i < mesh->nVertices; i++)
//	//	{
//	//		vertices[i] = *(Vec3*)((uint8_t*)mesh->vertexBuffers[0] + i * mesh->vertexSize);
//	//	}
//	//
//	//}
//	//
//	//if (mass == 0)
//	//	rigidEntity = physicsEngine->AddEntityRigidStatic(vertices, mesh->nVertices, mesh->indices, mesh->indexSize, mesh->nIndices);
//	//else
//	//	rigidEntity = physicsEngine->AddEntityRigidDynamic(vertices, mesh->nVertices, mass);
//	//
//	////loadedPhysicalVertexPositions = vertices;
//	////delete vertices;
//	////vertices = nullptr; // Important
//	//
//	//auto c = new RigidBodyPart(rigidEntity);
//	//parts.push_back(c);
//	//return c;
//}
//
//RigidBodyPart* Core::AddPart_RigidBodyCapsule(float height, float radius, float mass /* = 0*/)
//{
//	auto capsuleEntity = physicsEngine->AddEntityRigidCapsule(height, radius, mass);
//
//	auto c = new RigidBodyPart(capsuleEntity);
//	parts.push_back(c);
//	return c;
//}
//
//PerspCameraPart* Core::AddPart_Camera()
//{
//	auto c = new PerspCameraPart(graphicsEngine->CreatePerspectiveCamera(""));
//	parts.push_back(c);
//	return c;
//}
//
//Transform3DPart* Core::AddPart_Transform3D()
//{
//	auto c = new Transform3DPart();
//	parts.push_back(c);
//	return c;
//}
//

void Core::Update(float deltaTime)
{
	{
		PROFILE_SCOPE("Game Logic");

		// Scripts
		{
			PROFILE_SCOPE("Scripts");
			for (auto& s : scripts)
				s->Update(deltaTime);
		}


		//Entity Scripts
		{
			PROFILE_SCOPE("Entity Scripts");
			for (auto& s : actorScripts)
				s->Update(deltaTime);
		}


		// ActorLambda onUpdate
		{
			PROFILE_SCOPE("ActorLambda onUpdate");
			for (auto& a : actors)
				if (!a->IsKilled())
					a->Update(deltaTime);
		}

		// TODO optimize
		// Process Tasks if time passed, and remove after dispatch
		{
			PROFILE_SCOPE("AddTask Dispatch");

			static std::vector<size_t> indicesToDelete;
			indicesToDelete.clear();

			size_t oldSize = tasks.size(); // Cuz you can AddTask in AddTask ^^, tasks.size() can change in loop body
			for (size_t i = 0; i < oldSize; i++)
			{
				tasks[i].timeLeft -= deltaTime;

				if (tasks[i].timeLeft <= 0)
				{
					tasks[i].callb();
					indicesToDelete.push_back(i);
				}
			}

			// Remove dispatched taks
			for (size_t i = 0; i < indicesToDelete.size(); ++i)
				tasks.erase(tasks.begin() + indicesToDelete[i] - i);
		}
	}

	// Destroy actors queued for destroying
	//{
	//	PROFILE_SCOPE("Destroying Actors");
	//	for (auto& a : actorsToDestroy)
	//	{
	//		// Remove from actors
	//		auto it = std::find(actors.begin(), actors.end(), a);
	//		if (it != actors.end())
	//			actors.erase(it);
	//
	//		for (auto& Part : a->GetParts())
	//			RemovePart(Part);
	//
	//		delete a;
	//	}
	//	actorsToDestroy.clear();
	//
	//	PROFILE_SCOPE("Destroying Parts");
	//	for (auto& p : partsToDestroy)
	//	{
	//		RemovePart(p);
	//		delete p;
	//	}
	//	partsToDestroy.clear();
	//}

	//(RigidBody) Entity -> Part transform update
	//{
	//	PROFILE_SCOPE("(RigidBody) Entity -> Part transform update");
	//
	//	for (Part* part : GetParts(RIGID_BODY))
	//	{
	//		physics::IRigidBodyEntity* entity = ((RigidBodyPart*)part)->GetEntity();
	//		entity->SetPos(part->GetPos());
	//		entity->SetRot(part->GetRot());
	//		entity->SetScale(part->GetScale());
	//	}
	//}
	//
	//// (Mesh) Entity -> Part transform update
	//{
	//	PROFILE_SCOPE("(Mesh) Entity -> Part transform update");
	//
	//	for (Part* part : GetParts(MESH))
	//	{
	//		gxeng::MeshEntity* entity = ((MeshPart*)part)->GetEntity();
	//		auto pos = part->GetPos();
	//		auto rot = part->GetRot();
	//		auto scale = part->GetScale();
	//		entity->SetPosition(mathfu::Vector3f(pos.x, pos.y, pos.z));
	//		entity->SetRotation(mathfu::Quaternionf(rot.x, rot.y, rot.z, rot.w));
	//		entity->SetScale(mathfu::Vector3f(scale.x, scale.y, scale.z));
	//		//entity->SetSkew(c->GetSkew());
	//	}
	//}
	//
	//// (Camera) Entity -> Part transform update
	//{
	//	PROFILE_SCOPE("(Camera) Entity -> Part transform update");
	//
	//	for (Part* part : GetParts(CAMERA))
	//	{
	//		gxeng::PerspectiveCamera* cam = ((PerspCameraPart*)part)->GetCam();
	//
	//		// Position
	//		auto pos = part->GetPos();
	//		cam->SetPosition(mathfu::Vector3f(pos.x, pos.y, pos.z));
	//		
	//		// Rotation
	//		auto front = part->GetFrontDir();
	//		auto up = part->GetUpDir();
	//		cam->SetLookDirection(mathfu::Vector3f(front.x, front.y, front.z));
	//		cam->SetUpVector(mathfu::Vector3f(up.x, up.y, up.z));
	//		cam->SetTarget(mathfu::Vector3f(pos.x + front.x, pos.y + front.y, pos.z + front.z));
	//	}
	//}

	// Update physics
	if (physicsEngine)
	{
		PROFILE_SCOPE("Physics");
		physicsEngine->Update(deltaTime);
	}

	//Update rigid body components, from rigid body entity
	{
		PROFILE_SCOPE("Update rigid body components, from rigid body entity");
	
		for (Part* c : GetParts(RIGID_BODY))
		{
			physics::IRigidBodyEntity* entity = c->As<RigidBodyPart>()->GetEntity();
			c->SetPos(entity->GetPos());
			c->SetRot(entity->GetRot());
			c->SetScale(entity->GetScale());
		}
	}
	

	// Update graphics
	if (graphicsEngine)
	{
		PROFILE_SCOPE("Graphics");

//#ifdef PROFILE_ENGINE
//		graphicsEngine->GetGapi()->ResetStatesToDefault(); // Jesus the profiler also uses OpenGL temporarily, and mess up the binds etc...
//#endif
		graphicsEngine->Update(deltaTime);
	}

	if(guiEngine)
	{
		PROFILE_SCOPE("GUI");
		guiEngine->Update(deltaTime);
	}

	// Update sound
	//if (soundEngine)
	//{
	//	PROFILE_SCOPE("Sound");
	//	soundEngine->Update(deltaTime);
	//}
	//
	//// Update network
	//if (networkEngine)
	//{
	//	PROFILE_SCOPE("Network");
	//	networkEngine->Update(deltaTime);
	//}

	// Profiling engine
#ifdef PROFILE_ENGINE
	VisualCpuProfiler::UpdateAndPresent();
#endif
}

std::vector<Part*> Core::GetParts()
{
	return parts;
}

std::vector<Part*> Core::GetParts(ePartType type)
{
	std::vector<Part*> result;

	for (Part* c : GetParts())
		if (c->GetType() == type)
			result.push_back(c);

	return result;
}

} // namespace inl::core