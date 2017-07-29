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
	sun->SetColor({ 1.0f, 0.65f, 0.25f });
	sun->SetDirection({ 0.8f, -0.7f, -0.15f });
}

void Scene::Update()
{
	for (Part* part : parts)
		part->UpdateEntityTransform();
}

Scene::~Scene()
{
	for (Part* a : parts)
		delete a;

	parts.clear();
	actors.clear();
}

Actor* Scene::AddActor()
{
	return nullptr;// Core.AddActor();
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
		texture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR>::Reader());
	
		(*material)[0] = texture;
	}
	
	entity->SetMaterial(material);
	
	graphicsScene->GetMeshEntities().Add(entity);
	
	MeshActor* a = new MeshActor(entity);
	actors.push_back(a);
	parts.push_back(a);
	return a;
}

Actor* Scene::AddActor(const path& modelPath, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor(modelFilePath, mass);
}

Actor* Scene::AddActor_RigidBody(const path& modelPath, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor_RigidBody(modelFilePath, mass);
}

RigidBodyActor* Scene::AddActor_RigidBodyCapsule(float height, float radius, float mass /*= 0*/)
{
	return nullptr;//return Core.AddActor_RigidBodyCapsule(height, radius, mass);
}

PerspCameraActor* Scene::AddActor_PerspCamera()
{
	gxeng::PerspectiveCamera* cam = core->GetGraphicsEngine()->CreatePerspectiveCamera("WorldCam");
	PerspCameraActor* a = new PerspCameraActor(cam);
	actors.push_back(a);
	parts.push_back(a);
	return a;
}

DirectionalLightActor * Scene::AddActor_DirectionalLight()
{
	DirectionalLightActor* a = new DirectionalLightActor(graphicsScene);
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

bool Scene::TraceClosestPoint_Physics(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceInfo_out)
{
	return false;// Core.TraceClosestPoint_Physics(from, to, traceInfo_out);
}

bool Scene::TraceClosestPoint_Physics(PhysicsTraceResult& traceInfo_out)
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