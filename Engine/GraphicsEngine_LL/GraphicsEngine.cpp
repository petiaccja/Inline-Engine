#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include <BaseLibrary/Graph/Node.hpp>
#include <BaseLibrary/Graph/NodeLibrary.hpp>

#include <iostream> // only for debugging
#include <regex> // as well...
#include <lemon/bfs.h> // as well...

#include <rapidjson/document.h>
#include <optional>

#include "Nodes/Node_GetBackBuffer.hpp"
#include "Nodes/Node_TextureProperties.hpp"
#include "Nodes/Node_CreateTexture.hpp"
#include "Nodes/Node_GetSceneByName.hpp"
#include "Nodes/Node_GetCameraByName.hpp"
#include "Nodes/Node_GetCamera2DByName.hpp"
#include "Nodes/Node_GetTime.hpp"
#include "Nodes/Node_GetEnvVariable.hpp"
#include "Nodes/Node_VectorComponents.hpp"


//forward
#include "Nodes/Node_ForwardRender.hpp"
#include "Nodes/Node_DepthPrepass.hpp"
#include "Nodes/Node_DepthReduction.hpp"
#include "Nodes/Node_DepthReductionFinal.hpp"
#include "Nodes/Node_CSM.hpp"
#include "Nodes/Node_DrawSky.hpp"
#include "Nodes/Node_DebugDraw.hpp"
#include "Nodes/Node_LightCulling.hpp"
#include "Nodes/Node_BrightLumPass.hpp"
#include "Nodes/Node_LuminanceReduction.hpp"
#include "Nodes/Node_LuminanceReductionFinal.hpp"
#include "Nodes/Node_HDRCombine.hpp"
#include "Nodes/Node_BloomDownsample.hpp"
#include "Nodes/Node_BloomBlur.hpp"
#include "Nodes/Node_BloomAdd.hpp"
#include "Nodes/Node_TileMax.hpp"
#include "Nodes/Node_NeighborMax.hpp"
#include "Nodes/Node_MotionBlur.hpp"
#include "Nodes/Node_LensFlare.hpp"
#include "Nodes/Node_SMAA.hpp"
#include "Nodes/Node_DOFPrepare.hpp"
#include "Nodes/Node_DOFTileMax.hpp"
#include "Nodes/Node_DOFNeighborMax.hpp"
#include "Nodes/Node_DOFMain.hpp"
#include "Nodes/Node_Voxelization.hpp"
#include "Nodes/Node_VolumetricLighting.hpp"
#include "Nodes/Node_ShadowMapGen.hpp"
#include "Nodes/Node_ScreenSpaceShadow.hpp"
#include "Nodes/Node_ScreenSpaceReflection.hpp"
#include "Nodes/Node_ScreenSpaceAmbientOcclusion.hpp"

//Gui
#include "Nodes/Node_RenderOverlay.hpp"
#include "Nodes/Node_Blend.hpp"
#include "Nodes/Node_ScreenSpaceTransform.hpp"
#include "Nodes/Node_BlendWithTransform.hpp"

#include "Scene.hpp"
#include "PerspectiveCamera.hpp"
#include "OrthographicCamera.hpp"
#include "Camera2D.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Image.hpp"
#include "MeshEntity.hpp"
#include "OverlayEntity.hpp"
#include "Font.hpp"
#include "TextEntity.hpp"


namespace inl {
namespace gxeng {

using namespace gxapi;


GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc)
	: m_gxapiManager(desc.gxapiManager),
	m_graphicsApi(desc.graphicsApi),
	m_commandAllocatorPool(desc.graphicsApi),
	m_commandListPool(desc.graphicsApi),
	m_scratchSpacePool(desc.graphicsApi, gxapi::eDescriptorHeapType::CBV_SRV_UAV),
	m_textureSpace(desc.graphicsApi),
	m_masterCommandQueue(desc.graphicsApi->CreateCommandQueue(CommandQueueDesc{ eCommandListType::GRAPHICS }), desc.graphicsApi->CreateFence(0)),
	m_residencyQueue(std::unique_ptr<gxapi::IFence>(desc.graphicsApi->CreateFence(0))),
	m_memoryManager(desc.graphicsApi),
	m_dsvHeap(desc.graphicsApi),
	m_rtvHeap(desc.graphicsApi),
	m_persResViewHeap(desc.graphicsApi),
	m_logger(desc.logger),
	m_shaderManager(desc.gxapiManager)
{
	// Create swapchain
	SwapChainDesc swapChainDesc;
	swapChainDesc.format = eFormat::R8G8B8A8_UNORM;
	swapChainDesc.width = desc.width;
	swapChainDesc.height = desc.height;
	swapChainDesc.numBuffers = 2;
	swapChainDesc.targetWindow = desc.targetWindow;
	swapChainDesc.isFullScreen = desc.fullScreen;
	swapChainDesc.multisampleCount = 1;
	swapChainDesc.multiSampleQuality = 0;
	m_swapChain.reset(m_gxapiManager->CreateSwapChain(swapChainDesc, m_masterCommandQueue.GetUnderlyingQueue()));

	m_frameEndFenceValues.resize(m_swapChain->GetDesc().numBuffers, { nullptr, 0 });

	// Init backbuffer heap
	m_backBufferHeap = std::make_unique<BackBufferManager>(m_graphicsApi, m_swapChain.get());

	// Init shader manager before creating the pipeline
	gxapi::eShaderCompileFlags shaderFlags;
	shaderFlags += gxapi::eShaderCompileFlags::ROW_MAJOR_MATRICES;
#ifdef NDEBUG
	shaderFlags += gxapi::eShaderCompileFlags::OPTIMIZATION_HIGH;
#else
	shaderFlags += gxapi::eShaderCompileFlags::NO_OPTIMIZATION;
	shaderFlags += gxapi::eShaderCompileFlags::DEBUG;
#endif // NDEBUG
	m_shaderManager.SetShaderCompileFlags(shaderFlags);

	// Register nodes
	RegisterPipelineClasses();

	// Init logger
	m_logStreamGeneral = m_logger->CreateLogStream("General");
	m_logStreamPipeline = m_logger->CreateLogStream("Pipeline");

	// Init misc stuff
	m_absoluteTime = decltype(m_absoluteTime)(0);
	m_commandAllocatorPool.SetLogStream(&m_logStreamPipeline);

	m_pipelineEventDispatcher += &m_memoryManager.GetUploadManager();
	// DELETE THIS
	m_pipelineEventPrinter.SetLog(&m_logStreamPipeline);
	m_pipelineEventDispatcher += &m_pipelineEventPrinter;



	// Begin awaiting frame #0's Update()
	m_pipelineEventDispatcher.DispachFrameBeginAwait(0);
}


GraphicsEngine::~GraphicsEngine() {
	std::cout << "Graphics engine shutting down..." << std::endl;
	FlushPipelineQueue();
	std::cout << "Graphics engine deleting..." << std::endl;
}


void GraphicsEngine::Update(float elapsed) {
	std::chrono::nanoseconds frameTime(long long(elapsed * 1e9));
	m_absoluteTime += frameTime;

	// Wait for previous frame on this BB to complete
	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	if (m_frameEndFenceValues[backBufferIndex]) {
		m_frameEndFenceValues[backBufferIndex].Wait();
	}

	// Set up context
	FrameContext context;
	context.frameTime = frameTime;
	context.absoluteTime = m_absoluteTime;
	context.log = &m_logStreamPipeline;
	context.frame = m_frame;

	context.gxApi = m_graphicsApi;
	context.commandAllocatorPool = &m_commandAllocatorPool;
	context.commandListPool = &m_commandListPool;
	context.scratchSpacePool = &m_scratchSpacePool;
	context.memoryManager = &m_memoryManager;
	context.textureSpace = &m_textureSpace;
	context.rtvHeap = &m_rtvHeap;
	context.dsvHeap = &m_dsvHeap;
	context.shaderManager = &m_shaderManager;

	context.commandQueue = &m_masterCommandQueue;
	context.backBuffer = &m_backBufferHeap->GetBackBuffer(backBufferIndex);
	context.scenes = &m_scenes;
	context.cameras = &m_cameras;

	const std::vector<UploadManager::UploadDescription>& uploadRequests = m_memoryManager.GetUploadManager().GetQueuedUploads();
	context.uploadRequests = &uploadRequests;

	context.residencyQueue = &m_residencyQueue;

	// Update special nodes for current frame
	UpdateSpecialNodes();

	// Execute the pipeline
	m_pipelineEventDispatcher.DispatchFrameBegin(m_frame).wait();
	m_scheduler.Execute(context);
	m_pipelineEventDispatcher.DispatchFrameEnd(m_frame).wait();

	// Mark frame completion
	SyncPoint frameEnd = m_masterCommandQueue.Signal();
	m_frameEndFenceValues[backBufferIndex] = frameEnd;
	m_pipelineEventDispatcher.DispatchDeviceFrameEnd(frameEnd, m_frame);

	// Flush log
	m_logger->Flush();

	// Present frame
	m_swapChain->Present();
	++m_frame;

	// Await next frame
	m_pipelineEventDispatcher.DispachFrameBeginAwait(m_frame).wait(); // m_frame incremented on previous line
}


void GraphicsEngine::SetScreenSize(unsigned width, unsigned height) {
	if (width == 0 || height == 0) {
		return;
	}

	FlushPipelineQueue();

	m_backBufferHeap.reset();
	m_scheduler.ReleaseResources();
	m_swapChain->Resize(width, height);
	m_backBufferHeap = std::make_unique<BackBufferManager>(m_graphicsApi, m_swapChain.get());
}
void GraphicsEngine::GetScreenSize(unsigned& width, unsigned& height) {
	auto desc = m_swapChain->GetDesc();
	width = desc.width;
	height = desc.height;
}


void GraphicsEngine::SetFullScreen(bool enable) {
	m_swapChain->SetFullScreen(enable);
}
bool GraphicsEngine::GetFullScreen() const {
	return m_swapChain->IsFullScreen();
}


// Resources
Mesh* GraphicsEngine::CreateMesh() {
	return new Mesh(&m_memoryManager);
}

Image* GraphicsEngine::CreateImage() {
	return new Image(&m_memoryManager, &m_textureSpace);
}

Material* GraphicsEngine::CreateMaterial() {
	return new Material;
}

MaterialShaderEquation* GraphicsEngine::CreateMaterialShaderEquation() {
	return new MaterialShaderEquation(&m_shaderManager);
}

MaterialShaderGraph* GraphicsEngine::CreateMaterialShaderGraph() {
	return new MaterialShaderGraph(&m_shaderManager);
}

Font* GraphicsEngine::CreateFont() {
	return new Font(std::move(*CreateImage()));
}



// Scene
Scene* GraphicsEngine::CreateScene(std::string name) {
	// Declare a derived class for the sole purpose of making the destructor unregister the object from scene list.
	class ObservedScene : public Scene {
	public:
		ObservedScene(std::function<void(Scene*)> deleteHandler, std::string name) :
			Scene(std::move(name)), m_deleteHandler(std::move(deleteHandler))
		{}
		~ObservedScene() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<Scene*>(this)); }
		}
	protected:
		std::function<void(Scene*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterScene = [this](Scene* arg) {
		m_scenes.erase(arg);
	};

	// Allocate a new scene, and register it.
	Scene* scene = new ObservedScene(unregisterScene, std::move(name));
	m_scenes.insert(scene);

	return scene;
}


PerspectiveCamera* GraphicsEngine::CreatePerspectiveCamera(std::string name) {
	class ObservedPerspectiveCamera : public PerspectiveCamera {
	public:
		ObservedPerspectiveCamera(std::function<void(PerspectiveCamera*)> deleteHandler, std::string name) :
			m_deleteHandler(std::move(deleteHandler))
		{
			SetName(name);
		}
		~ObservedPerspectiveCamera() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<PerspectiveCamera*>(this)); }
		}
	protected:
		std::function<void(PerspectiveCamera*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterCamera = [this](PerspectiveCamera* arg) {
		m_cameras.erase(arg);
	};

	// Allocate a new scene, and register it.
	PerspectiveCamera* camera = new ObservedPerspectiveCamera(unregisterCamera, std::move(name));
	m_cameras.insert(camera);

	return camera;
}

OrthographicCamera* GraphicsEngine::CreateOrthographicCamera(std::string name) {
	class ObservedOrthographicCamera : public OrthographicCamera {
	public:
		ObservedOrthographicCamera(std::function<void(OrthographicCamera*)> deleteHandler, std::string name) :
			m_deleteHandler(std::move(deleteHandler)) {
			SetName(name);
		}
		~ObservedOrthographicCamera() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<OrthographicCamera*>(this)); }
		}
	protected:
		std::function<void(OrthographicCamera*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterCamera = [this](OrthographicCamera* arg) {
		m_cameras.erase(arg);
	};

	// Allocate a new scene, and register it.
	OrthographicCamera* camera = new ObservedOrthographicCamera(unregisterCamera, std::move(name));
	m_cameras.insert(camera);

	return camera;
}

Camera2D* GraphicsEngine::CreateCamera2D(std::string name) {
	class ObservedCamera2D : public Camera2D {
	public:
		ObservedCamera2D(std::function<void(Camera2D*)> deleteHandler, std::string name) :
			m_deleteHandler(std::move(deleteHandler)) {
			SetName(name);
		}
		~ObservedCamera2D() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<Camera2D*>(this)); }
		}
	protected:
		std::function<void(Camera2D*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterCamera = [this](Camera2D* arg) {
		m_cameras2d.erase(arg);
	};

	// Allocate a new scene, and register it.
	Camera2D* camera = new ObservedCamera2D(unregisterCamera, std::move(name));
	m_cameras2d.insert(camera);

	return camera;
}

MeshEntity* GraphicsEngine::CreateMeshEntity() {
	return new MeshEntity;
}

OverlayEntity* GraphicsEngine::CreateOverlayEntity() {
	return new OverlayEntity;
}

TextEntity* GraphicsEngine::CreateTextEntity() {
	return new TextEntity;
}


bool GraphicsEngine::SetEnvVariable(std::string name, Any obj) {
	auto res = m_envVariables.insert_or_assign(std::move(name), std::move(obj));
	return res.second;
}

bool GraphicsEngine::EnvVariableExists(const std::string& name) {
	return m_envVariables.count(name) > 0;
}

const Any& GraphicsEngine::GetEnvVariable(const std::string& name) {
	auto it = m_envVariables.find(name);
	if (it != m_envVariables.end()) {
		return it->second;
	}
	else {
		throw InvalidArgumentException("Environment variable does not exist.");
	}
}


void GraphicsEngine::LoadPipeline(const std::string& graphDesc) {
	FlushPipelineQueue();

	Pipeline pipeline;
	pipeline.CreateFromDescription(graphDesc, m_nodeFactory);

	EngineContext engineContext(1, 1);
	for (auto& node : pipeline) {
		if (auto graphicsNode = dynamic_cast<GraphicsNode*>(&node)) {
			graphicsNode->Initialize(engineContext);
		}
	}

	auto specialNodes = SelectSpecialNodes(pipeline);

	m_specialNodes = specialNodes;
	m_pipeline = std::move(pipeline);
	DumpPipelineGraph(m_pipeline, "pipeline_graph.dot");
	m_scheduler.SetPipeline(std::move(m_pipeline));
}


void GraphicsEngine::SetShaderDirectories(const std::vector<std::experimental::filesystem::path>& directories) {
	m_shaderManager.ClearSourceDirectories();
	for (auto directory : directories) {
		m_shaderManager.AddSourceDirectory(directory);
	}
}


void GraphicsEngine::FlushPipelineQueue() {
	SyncPoint lastSync = m_masterCommandQueue.Signal();
	lastSync.Wait();
}


std::vector<GraphicsNode*> GraphicsEngine::SelectSpecialNodes(Pipeline& pipeline) {
	std::vector<GraphicsNode*> specialNodes;

	for (NodeBase& node : pipeline) {
		// Pipeline disallows linking of its nodes, that's why it only returns const pointers.
		// We are not changing linking configuration here, so const_cast is justified.
		if (nodes::GetSceneByName* ptr = dynamic_cast<nodes::GetSceneByName*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetCameraByName* ptr = dynamic_cast<nodes::GetCameraByName*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetCamera2DByName* ptr = dynamic_cast<nodes::GetCamera2DByName*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetBackBuffer* ptr = dynamic_cast<nodes::GetBackBuffer*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetTime* ptr = dynamic_cast<nodes::GetTime*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetEnvVariable* ptr = dynamic_cast<nodes::GetEnvVariable*>(&node)) {
			specialNodes.push_back(ptr);
		}
	}

	return specialNodes;
}


void GraphicsEngine::UpdateSpecialNodes() {
	std::vector<const Scene*> scenes;
	for (auto scene : m_scenes) {
		scenes.push_back(scene);
	}

	std::vector<const BasicCamera*> cameras;
	for (auto camera : m_cameras) {
		cameras.push_back(camera);
	}

	std::vector<const Camera2D*> cameras2d;
	for (auto camera : m_cameras2d) {
		cameras2d.push_back(camera);
	}

	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	Texture2D backBuffer = m_backBufferHeap->GetBackBuffer(backBufferIndex).GetResource();

	for (auto node : m_specialNodes) {
		if (auto* getScene = dynamic_cast<nodes::GetSceneByName*>(node)) {
			getScene->SetSceneList(scenes);
		}
		else if (auto* getCamera = dynamic_cast<nodes::GetCameraByName*>(node)) {
			getCamera->SetCameraList(cameras);
		}
		else if (auto* getCamera = dynamic_cast<nodes::GetCamera2DByName*>(node)) {
			getCamera->SetCameraList(cameras2d);
		}
		else if (auto* getBB = dynamic_cast<nodes::GetBackBuffer*>(node)) {
			getBB->SetBuffer(backBuffer);
		}
		else if (auto* getTime = dynamic_cast<nodes::GetTime*>(node)) {
			getTime->SetAbsoluteTime(m_absoluteTime.count() / 1e9);
		}
		else if (auto* getEnv = dynamic_cast<nodes::GetEnvVariable*>(node)) {
			getEnv->SetEnvVariableList(&m_envVariables);
		}
	}
}


void GraphicsEngine::RegisterPipelineClasses() {
	RegisterIntegerArithmeticNodes(&m_nodeFactory, "Integer");
	RegisterIntegerComparisonNodes(&m_nodeFactory, "Integer");
	RegisterFloatArithmeticNodes(&m_nodeFactory, "Float");
	RegisterFloatComparisonNodes(&m_nodeFactory, "Float");
	RegisterFloatMathNodes(&m_nodeFactory, "Float");
	RegisterLogicNodes(&m_nodeFactory, "Logic");


	m_nodeFactory.RegisterNodeClass<nodes::GetBackBuffer>("Pipeline/System");
	m_nodeFactory.RegisterNodeClass<nodes::GetSceneByName>("Pipeline/System");
	m_nodeFactory.RegisterNodeClass<nodes::GetCameraByName>("Pipeline/System");
	m_nodeFactory.RegisterNodeClass<nodes::GetCamera2DByName>("Pipeline/System");
	m_nodeFactory.RegisterNodeClass<nodes::GetTime>("Pipeline/System");
	m_nodeFactory.RegisterNodeClass<nodes::GetEnvVariable>("Pipeline/System");

	m_nodeFactory.RegisterNodeClass<nodes::TextureProperties>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::CreateTexture>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::VectorComponents<1>>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::VectorComponents<2>>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::VectorComponents<3>>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::VectorComponents<4>>("Pipeline/Utility");

	m_nodeFactory.RegisterNodeClass<nodes::ForwardRender>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DepthPrepass>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DepthReduction>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DepthReductionFinal>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::CSM>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DrawSky>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DebugDraw>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::LightCulling>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::BrightLumPass>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::LuminanceReduction>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::LuminanceReductionFinal>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::HDRCombine>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::BloomDownsample>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::BloomBlur>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::BloomAdd>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::TileMax>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::NeighborMax>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::MotionBlur>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::LensFlare>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::SMAA>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DOFPrepare>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DOFTileMax>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DOFNeighborMax>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::DOFMain>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::Voxelization>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::VolumetricLighting>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::ShadowMapGen>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::ScreenSpaceShadow>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::ScreenSpaceReflection>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::ScreenSpaceAmbientOcclusion>("Pipeline/Render");

	m_nodeFactory.RegisterNodeClass<nodes::RenderOverlay>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::Blend>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::ScreenSpaceTransform>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::BlendWithTransform>("Pipeline/Render");
}



std::string TidyTypeName(std::string name) {
	std::string s2;

	// Replace <> with &lt; and &gt; escapes
	int inTemplate = 0;
	for (auto c : name) {
		if (c == '<') {
			inTemplate++;
			s2 += "&lt;";
		}
		else if (c == '>') {
			inTemplate--;
			s2 += "&gt;";
		}
		else {
			s2 += c;
		}
	}

	std::vector<std::string> stripNamespaces = {
		"inl::gxeng::",
		"inl::",
		"mathter::",
		"nodes::",
	};

	// Remove class, struct and enum specifiers.
	std::regex classFilter(R"(\s*class\s*)");
	s2 = std::regex_replace(s2, classFilter, "");

	std::regex structFilter(R"(\s*struct\s*)");
	s2 = std::regex_replace(s2, structFilter, "");

	std::regex enumFilter(R"(\s*enum\s*)");
	s2 = std::regex_replace(s2, enumFilter, "");

	// MSVC specific things.
	std::regex ptrFilter(R"(\s*__ptr64\s*)");
	s2 = std::regex_replace(s2, ptrFilter, "");

	// Transform common templates to readable format
	std::regex stringFilter(R"(\s*std::basic_string.*)");
	s2 = std::regex_replace(s2, stringFilter, "std::string");

	// Remove consts.
	std::regex constFilter(R"(\s*const\s*)");
	s2 = std::regex_replace(s2, constFilter, "");

	// Remove requested s2spaces.
	for (auto& ns : stripNamespaces) {
		std::regex s2spaceFilter1(R"(\s*)" + ns + R"(\s*)");
		s2 = std::regex_replace(s2, s2spaceFilter1, "");
	}

	return s2;
}


void GraphicsEngine::DumpPipelineGraph(const Pipeline& pipeline, std::string file) {
	std::stringstream dot; // graphviz dot file

	struct PortMap {
		const NodeBase* parent;
		int portIndex;
	};

	std::map<const InputPortBase*, PortMap> inputParents;
	std::map<const OutputPortBase*, PortMap> outputParents;
	std::map<const NodeBase*, size_t> nodeIndexMap;

	// Fill node map and parent maps
	for (const NodeBase& node : pipeline) {
		size_t nodeIndex = nodeIndexMap.size();
		nodeIndexMap.insert({ &node, nodeIndex });

		for (int i = 0; i < node.GetNumInputs(); ++i) {
			inputParents.insert({ node.GetInput(i), PortMap{&node, i} });
		}
		for (int i = 0; i < node.GetNumOutputs(); ++i) {
			outputParents.insert({ node.GetOutput(i), PortMap{ &node, i } });
		}
	}

	// Write out preamble
	dot << "digraph structs {" << std::endl;
	dot << "rankdir=LR;" << std::endl;
	dot << "ranksep=\"0.8\";" << std::endl;
	dot << "node [shape=record];" << std::endl;
	dot << std::endl;

	// Write out nodes
	for (const auto& v : nodeIndexMap) {
		dot << "node" << v.second << " [shape=record, label=\"";
		//dot << "&lt;&lt;&lt; " << TidyTypeName(typeid(*v.first).name()) << " &gt;&gt;&gt;";
		dot << v.first->GetDisplayName() << " : " << TidyTypeName(v.first->GetClassName(true, {"inl::gxeng::", "inl::"}));
		dot << " | {";
		// Inputs
		dot << "{";
		for (int i = 0; i < v.first->GetNumInputs(); ++i) {
			const InputPortBase* port = v.first->GetInput(i);
			dot << "<in" << i << "> ";
			dot << TidyTypeName(port->GetType().name());
			if (i < (int)v.first->GetNumInputs() - 1) {
				dot << " | ";
			}
		}
		dot << "} | ";
		// Outputs
		dot << "{";
		for (int i = 0; i < v.first->GetNumOutputs(); ++i) {
			const OutputPortBase* port = v.first->GetOutput(i);
			dot << "<out" << i << "> ";
			dot << TidyTypeName(port->GetType().name());
			if (i < (int)v.first->GetNumOutputs() - 1) {
				dot << " | ";
			}
		}
		dot << "}";

		dot << "}\"];" << std::endl;
	}

	dot << std::endl;

	// Write out links
	for (const auto& v : nodeIndexMap) {
		// Inputs
		for (int i = 0; i < v.first->GetNumInputs(); ++i) {
			const InputPortBase* target = v.first->GetInput(i);
			OutputPortBase* source = target->GetLink();
			if (source && outputParents.count(source) > 0) {
				auto srcNode = outputParents[source];
				auto tarNode = inputParents[target];

				dot << "node" << nodeIndexMap[srcNode.parent] << ":";
				dot << "out" << srcNode.portIndex;
				dot << " -> ";
				dot << "node" << nodeIndexMap[tarNode.parent] << ":";
				dot << "in" << tarNode.portIndex;
				dot << ";" << std::endl;
				assert(tarNode.portIndex == i);
			}
		}
	}

	dot << std::endl;

	// Write closing
	dot << "}" << std::endl;


	// Write out file
	std::ofstream f(file, std::ios::trunc);
	if (f.is_open()) {
		f << dot.str();
	}
}



} // namespace gxeng
} // namespace inl