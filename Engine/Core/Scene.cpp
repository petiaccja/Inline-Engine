#include "Scene.hpp"
#include "Core.hpp"

#include <AssetLibrary/Model.hpp>
#include <AssetLibrary/Image.hpp>
#include <GraphicsEngine_LL/Pixel.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/Image.hpp>

namespace inl::core {

using namespace inl::asset;

Scene::Scene(Core* core)
:core(core)
{
	graphicsScene = core->GetGraphicsEngine()->CreateScene("World");
	physicsScene = core->GetPhysicsEngine()->CreateScene();

	DirectionalLightActor* sun = AddActor_DirectionalLight();

	sun->SetColor({ 1.0f, 0.9f, 0.85f });
	sun->SetDirection({ 0.8f, -0.7f, -0.9f });
}

void Scene::Update(float deltaTime)
{
	for (Part* part : parts)
		part->UpdateEntityTransform();
	
	if (physicsScene)
		physicsScene->Update(deltaTime);
}

bool Scene::TraceGraphicsRay(const Ray3D& ray, TraceResult& traceResult_out)
{
	for (int i = 0; i < 10; ++i)
	{
		MeshActor* mesh = this->AddActor_Mesh("D:/sphere2.fbx");
		mesh->SetScale({ 0.01, 0.01, 0.01 });
	
		mesh->SetPos(ray.base + i * ray.direction * 15.0f);
	}
	
	// Temporarily graphics tracing is solved with physics
	physics::TraceResult result;
	if (physicsScene->TraceRay(ray, result))
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

Scene::~Scene()
{
	for (Part* a : parts)
		delete a;

	parts.clear();
	actors.clear();
}

EmptyActor* Scene::AddActor()
{
	EmptyActor* a = new EmptyActor(this);

	actors.push_back(a);
	parts.push_back(a);
	return a;
}

void Scene::AddActor(Actor* a)
{
	actors.push_back(a);
	parts.push_back(a);
}

MeshActor* Scene::AddActor_Mesh(const path& modelPath)
{
	gxeng::GraphicsEngine* graphicsEngine = core->GetGraphicsEngine();
	
	auto str = modelPath.generic_string();
	std::wstring path = modelPath;
	Model* model = new Model(std::string(path.begin(), path.end()));
	
	inl::asset::CoordSysLayout coordSysLayout = { AxisDir::POS_X,   AxisDir::NEG_Z , AxisDir::NEG_Y };
	
	auto modelVertices = model->GetVertices<gxeng::Position<0>, gxeng::Normal<0>, gxeng::TexCoord<0>>(0, coordSysLayout);
	std::vector<unsigned> modelIndices = model->GetIndices(0);
	
	gxeng::Mesh* mesh = graphicsEngine->CreateMesh();
	mesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	
	gxeng::MeshEntity* entity = new gxeng::MeshEntity();
	entity->SetMesh(mesh);
	
	// Create material
	gxeng::Material* material = graphicsEngine->CreateMaterial();
	
	gxeng::MaterialShaderGraph* graph = graphicsEngine->CreateMaterialShaderGraph();
	
	std::unique_ptr<inl::gxeng::MaterialShaderEquation> mapShader(graphicsEngine->CreateMaterialShaderEquation());
	std::unique_ptr<inl::gxeng::MaterialShaderEquation> diffuseShader(graphicsEngine->CreateMaterialShaderEquation());
	
	mapShader->SetSourceName("bitmap_color_2d.mtl");
	diffuseShader->SetSourceName("simple_diffuse.mtl");
	
	std::vector<std::unique_ptr<inl::gxeng::MaterialShader>> nodes;
	nodes.push_back(std::move(mapShader));
	nodes.push_back(std::move(diffuseShader));
	graph->SetGraph(std::move(nodes), { { 0, 1, 0 } });
	material->SetShader(graph);
	
	// Create texture
	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR>;
		static inl::asset::Image img("assets\\pine_tree.jpg");
	
		gxeng::Image* texture = graphicsEngine->CreateImage();
			
		texture->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR);
		texture->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		(*material)[0] = texture;
	}
	
	entity->SetMaterial(material);
	
	graphicsScene->GetMeshEntities().Add(entity);
	
	MeshActor* a = new MeshActor(this, entity);
	actors.push_back(a);
	parts.push_back(a);
	return a;
}

RigidBodyActor* Scene::AddActor_RigidBody(const path& modelPath, float mass /*= 0*/)
{
	// Load model
	Model model(modelPath.generic_string());

	inl::asset::CoordSysLayout coordSysLayout = { AxisDir::POS_X,   AxisDir::NEG_Z , AxisDir::NEG_Y };
	
	auto modelVertices = model.GetVertices<gxeng::Position<0>, gxeng::Normal<0>, gxeng::TexCoord<0>>(0, coordSysLayout);
	std::vector<unsigned> modelIndices = model.GetIndices(0);
	
	
	physics::IRigidBodyEntity* rigidBodyEntity = nullptr;
	
	std::vector<unsigned> tmp = modelIndices;
	
	unsigned* indices = tmp.data();
	int vertexCount  = modelVertices.size();
	int indexCount = tmp.size();
	int indexSize = sizeof(unsigned);
	
	Vec3* vertices = new Vec3[vertexCount];
	
	for (int i = 0; i < vertexCount; ++i)
	{
		auto& vertex = modelVertices[i];
		vertices[i] = vertex.position;
	}
	
	if (mass != 0)
		rigidBodyEntity = physicsScene->AddEntityRigidDynamic(vertices, vertexCount, mass);
	else
		rigidBodyEntity = physicsScene->AddEntityRigidStatic(vertices, vertexCount, indices, indexSize, indexCount);
	
	delete vertices;
	
	RigidBodyActor* actor = new RigidBodyActor(this, rigidBodyEntity);
	parts.push_back(actor);
	actors.push_back(actor);

	return actor;
}

RigidBodyActor* Scene::AddActor_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor_RigidBodyCapsule(height, radius, mass);
}

PerspCameraActor* Scene::AddActor_PerspCamera()
{
	gxeng::PerspectiveCamera* cam = core->GetGraphicsEngine()->CreatePerspectiveCamera("WorldCam");
	PerspCameraActor* a = new PerspCameraActor(this, cam);
	actors.push_back(a);
	parts.push_back(a);
	return a;
}

DirectionalLightActor * Scene::AddActor_DirectionalLight()
{
	DirectionalLightActor* a = new DirectionalLightActor(this, graphicsScene);
	actors.push_back(a);
	parts.push_back(a);
	return a;
}

MeshPart* Scene::CreatePart_Mesh(const std::string& modelFilePath)
{
	return nullptr;//return Core.AddPart_Mesh(modelFilePath);
}

RigidBodyPart* Scene::CreatePart_RigidBody(const std::string& modelFilePath, float mass)
{
	return nullptr;//return Core.AddPart_RigidBody(modelFilePath, mass);
}

RigidBodyPart* Scene::CreatePart_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	return nullptr;//return Core.AddPart_RigidBodyCapsule(height, radius, mass);
}
PerspCameraPart* Scene::CreatePart_Camera()
{
	return nullptr;//return Core.AddPart_Camera();
}

void Scene::RemoveActor(Actor* a)
{
	return;// Core.RemoveActor(a);
}

void Scene::DestroyPart(Part* c)
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

void Scene::SetLayerCollision(uint64_t ID0, uint64_t ID1, bool bEnableCollision)
{
	physicsScene->SetLayerCollision(ID0, ID1, bEnableCollision);
	//assert(physicsEngine);
	//physicsEngine->SetLayerCollision(ID0, ID1, bEnableCollision);
}

} // namespace inl::core