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
#include "Nodes/Node_TextRender.hpp"
#include "Nodes/Node_ScreenSpaceAmbientOcclusion.hpp"

//Gui
#include "Nodes/Node_OverlayRender.hpp"
#include "Nodes/Node_Blend.hpp"
#include "Nodes/Node_ScreenSpaceTransform.hpp"
#include "Nodes/Node_BlendWithTransform.hpp"

#include "Scene.hpp"
#include "PerspectiveCamera.hpp"
#include "OrthographicCamera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Image.hpp"
#include "MeshEntity.hpp"
#include "OverlayEntity.hpp"


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
	m_shaderManager.AddSourceDirectory("../../Engine/GraphicsEngine_LL/Nodes/Shaders");
	m_shaderManager.AddSourceDirectory("../../Engine/GraphicsEngine_LL/Materials");
	m_shaderManager.AddSourceDirectory("./Shaders");
	m_shaderManager.AddSourceDirectory("./Materials");
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

	// Do more stuff...
	CreatePipeline();
	m_scheduler.SetPipeline(std::move(m_pipeline));

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
	SyncPoint lastSync = m_masterCommandQueue.Signal();
	lastSync.Wait();
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

	SyncPoint sp = m_masterCommandQueue.Signal();
	sp.Wait();

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

MeshEntity* GraphicsEngine::CreateMeshEntity() {
	return new MeshEntity;
}

OverlayEntity* GraphicsEngine::CreateOverlayEntity() {
	return new OverlayEntity;
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
	using namespace rapidjson;

	// Helper functions and structures.
	struct NodeCreationInfo {
		std::optional<int> id;
		std::optional<std::string> name;
		std::string cl;
		std::vector<std::optional<std::string>> inputs;
	};

	struct LinkCreationInfo {
		std::optional<int> srcid, dstid;
		std::optional<std::string> srcname, dstname;
		std::optional<int> srcpidx, dstpidx;
		std::optional<std::string> srcpname, dstpname;
	};

	auto AssertThrow = [](bool value, const std::string& text) {
		if (!value) {
			throw InvalidArgumentException(text);
		}
	};

	auto ParseNode = [&AssertThrow](const GenericValue<UTF8<>>& obj) {
		NodeCreationInfo info;
		if (obj.HasMember("id")) {
			AssertThrow(obj["id"].IsInt(), "Node's id member must be an integer.");
			info.id = obj["id"].GetInt();
		}
		if (obj.HasMember("name")) {
			AssertThrow(obj["name"].IsString(), "Node's name member must be a string.");
			info.name = obj["name"].GetString();
		}
		AssertThrow(info.id || info.name, "Node must have either id or name.");
		AssertThrow(obj.HasMember("class") && obj["class"].IsString(), "Node must have a class.");
		info.cl = obj["class"].GetString();

		if (obj.HasMember("inputs")) {
			AssertThrow(obj["inputs"].IsArray(), "Default inputs must be specified in an array, undefined inputs as {}.");
			auto& inputs = obj["inputs"];
			for (SizeType i = 0; i < inputs.Size(); ++i) {
				if (inputs[i].IsObject() && inputs[i].ObjectEmpty()) {
					info.inputs.push_back({});
				}
				else if (inputs[i].IsString()) {
					info.inputs.push_back(inputs[i].GetString());
				}
				else if (inputs[i].IsInt64()) {
					info.inputs.push_back(std::to_string(inputs[i].GetInt64()));
				}
				else if (inputs[i].IsDouble()) {
					info.inputs.push_back(std::to_string(inputs[i].GetDouble()));
				}
				else {
					assert(false);
				}
			}
		}
		return info;
	};

	auto ParseLink = [&AssertThrow](const GenericValue<UTF8<>>& obj) {
		LinkCreationInfo info;

		AssertThrow(obj.HasMember("src")
			&& obj.HasMember("dst")
			&& obj.HasMember("srcp")
			&& obj.HasMember("dstp"),
			"Link must have members src, dst, srcp and dstp.");

		if (obj["src"].IsString()) {
			info.srcname = obj["src"].GetString();
		}
		else if (obj["src"].IsInt()) {
			info.srcid = obj["src"].GetInt();
		}
		else {
			AssertThrow(false, "Link src must be string (name) or int (id) of the node.");
		}

		if (obj["dst"].IsString()) {
			info.dstname = obj["dst"].GetString();
		}
		else if (obj["dst"].IsInt()) {
			info.dstid = obj["dst"].GetInt();
		}
		else {
			AssertThrow(false, "Link dst must be string (name) or int (id) of the node.");
		}

		if (obj["srcp"].IsString()) {
			info.srcpname = obj["srcp"].GetString();
		}
		else if (obj["srcp"].IsInt()) {
			info.srcpidx = obj["srcp"].GetInt();
		}
		else {
			AssertThrow(false, "Link srcp must be string (name) or int (index) of the port.");
		}

		if (obj["dstp"].IsString()) {
			info.dstpname = obj["dstp"].GetString();
		}
		else if (obj["dstp"].IsInt()) {
			info.dstpidx = obj["dstp"].GetInt();
		}
		else {
			AssertThrow(false, "Link dstp must be string (name) or int (index) of the port.");
		}

		return info;
	};


	// Parse the JSON file.
	Document doc;
	doc.Parse(graphDesc.c_str());
	ParseErrorCode ec = doc.GetParseError();
	if (ec != ParseErrorCode::kParseErrorNone) {
		int ch = doc.GetErrorOffset();
		int chi = 0;
		int chsum = 0;
		int line = 0;
		while (chi < ch && chi < graphDesc.size()) {
			if (graphDesc[chi] == '\n') { ++line; chsum = 0; }
			++chi;
			++chsum;
		}
		throw InvalidArgumentException("JSON descripion has syntax errors.", "Check line " + std::to_string(line) + ":" + std::to_string(chsum));
	}

	AssertThrow(doc.IsObject(), "JSON root must be an object with member arrays \"nodes\" and \"links\".");
	AssertThrow(doc.HasMember("nodes") && doc["nodes"].IsArray(), "JSON root must have \"nodes\" member array.");
	AssertThrow(doc.HasMember("links") && doc["links"].IsArray(), "JSON root must have \"links\" member array.");

	auto& nodes = doc["nodes"];
	auto& links = doc["links"];
	std::vector<NodeCreationInfo> nodeCreationInfos;
	std::vector<LinkCreationInfo> linkCreationInfos;

	for (SizeType i = 0; i < nodes.Size(); ++i) {
		NodeCreationInfo info = ParseNode(nodes[i]);
		nodeCreationInfos.push_back(info);
	}

	for (SizeType i = 0; i < links.Size(); ++i) {
		LinkCreationInfo info = ParseLink(links[i]);
		linkCreationInfos.push_back(info);
	}

	// Create lookup dictionary of nodes by name and by id.
	// {name/id of node, index of node in vector}
	std::unordered_map<int, size_t> idBook;
	std::unordered_map<std::string, size_t> nameBook;
	for (size_t i = 0; i < nodeCreationInfos.size(); ++i) {
		if (nodeCreationInfos[i].name) {
			auto ins = nameBook.insert({ nodeCreationInfos[i].name.value(), i });
			AssertThrow(ins.second == true, "Node names must be unique.");
		}
		if (nodeCreationInfos[i].id) {
			auto ins = idBook.insert({ nodeCreationInfos[i].id.value(), i });
			AssertThrow(ins.second == true, "Node ids must be unique.");
		}
	}

	// Create nodes with initial values.
	std::vector<std::shared_ptr<NodeBase>> nodeObjects;
	for (auto& info : nodeCreationInfos) {
		std::shared_ptr<NodeBase> nodeObject(m_nodeFactory.CreateNode(info.cl));
		if (info.name) {
			nodeObject->SetDisplayName(info.name.value());
		}

		for (int i = 0; i < nodeObject->GetNumInputs() && i < info.inputs.size(); ++i) {
			if (info.inputs.size()) {
				nodeObject->GetInput(i)->SetConvert(info.inputs[i].value());
			}
		}

		nodeObjects.push_back(std::move(nodeObject));
	}

	// Link nodes above.
	for (auto& info : linkCreationInfos) {
		NodeBase *src, *dst;
		OutputPortBase* srcp;
		InputPortBase* dstp;

		// Find src and dst nodes
		if (info.srcname) {
			auto it = nameBook.find(info.srcname.value());
			AssertThrow(it != nameBook.end(), "Node requested to link named " + info.srcname.value() + " not found.");
			src = nodeObjects[it->second].get();
		}
		else {
			auto it = idBook.find(info.srcid.value());
			AssertThrow(it != idBook.end(), "Node requested to link id=" + std::to_string(info.srcid.value()) + " not found.");
			src = nodeObjects[it->second].get();
		}
		if (info.dstname) {
			auto it = nameBook.find(info.dstname.value());
			AssertThrow(it != nameBook.end(), "Node requested to link named " + info.dstname.value() + " not found.");
			dst = nodeObjects[it->second].get();
		}
		else {
			auto it = idBook.find(info.dstid.value());
			AssertThrow(it != idBook.end(), "Node requested to link id=" + std::to_string(info.dstid.value()) + " not found.");
			dst = nodeObjects[it->second].get();
		}
		// Find src and dst ports
		if (info.srcpname) {
			for (int i = 0; i < src->GetNumOutputs(); ++i) {
				if (info.srcpname.value() == src->GetOutputName(i)) {
					srcp = src->GetOutput(i);
					break;
				}
			}
		}
		else {
			srcp = src->GetOutput(info.srcpidx.value());
		}
		if (info.dstpname) {
			for (int i = 0; i < dst->GetNumInputs(); ++i) {
				if (info.dstpname.value() == dst->GetInputName(i)) {
					dstp = dst->GetInput(i);
					break;
				}
			}
		}
		else {
			dstp = dst->GetInput(info.dstpidx.value());
		}
		// Link said ports
		bool linked = srcp->Link(dstp);
		AssertThrow(linked, "Ports not compatible.");
	}


	// Finish off by creating the actual pipeline.
	EngineContext engineContext(1, 1);
	for (auto& node : nodeObjects) {
		if (auto graphicsNode = dynamic_cast<GraphicsNode*>(node.get())) {
			graphicsNode->Initialize(engineContext);
		}
	}

	Pipeline pipeline;
	pipeline.CreateFromNodesList(nodeObjects);

	DumpPipelineGraph(pipeline, "pipeline_graph_json.dot");
}



void GraphicsEngine::CreatePipeline() {
	auto swapChainDesc = m_swapChain->GetDesc();

	// -----------------------------
	// 3D pipeline path
	// -----------------------------
	std::shared_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::shared_ptr<nodes::GetCameraByName> getCamera(new nodes::GetCameraByName());
	std::shared_ptr<nodes::GetBackBuffer> getBackBuffer(new nodes::GetBackBuffer());
	
	getWorldScene->GetInput<0>().Set("World");
	getCamera->GetInput<0>().Set("WorldCam");

	std::shared_ptr<nodes::GetEnvVariable> getWorldRenderPos(new nodes::GetEnvVariable());
	std::shared_ptr<nodes::GetEnvVariable> getWorldRenderRot(new nodes::GetEnvVariable());
	std::shared_ptr<nodes::GetEnvVariable> getWorldRenderSize(new nodes::GetEnvVariable());
	std::shared_ptr<nodes::VectorComponents<2>> worldRenderSizeSplit(new nodes::VectorComponents<2>());

	getWorldRenderPos->SetEnvVariableList(&m_envVariables);
	getWorldRenderRot->SetEnvVariableList(&m_envVariables);
	getWorldRenderSize->SetEnvVariableList(&m_envVariables);

	getWorldRenderPos->GetInput<0>().Set("world_render_pos");
	getWorldRenderRot->GetInput<0>().Set("world_render_rot");
	getWorldRenderSize->GetInput<0>().Set("world_render_size");

	// TODO set the render buffers size to match "world_render_size"
	if (!worldRenderSizeSplit->GetInput<0>().Link(getWorldRenderSize->GetOutput(0))) 		{
		assert(false);
	}

	std::shared_ptr<nodes::TextureProperties> backBufferProperties(new nodes::TextureProperties());
	std::shared_ptr<nodes::CreateTexture> createDepthBuffer(new nodes::CreateTexture());
	std::shared_ptr<nodes::CreateTexture> createHdrRenderTarget(new nodes::CreateTexture());
	std::shared_ptr<nodes::CreateTexture> createCsmTextures(new nodes::CreateTexture());
	std::shared_ptr<nodes::ForwardRender> forwardRender(new nodes::ForwardRender());
	std::shared_ptr<nodes::DepthPrepass> depthPrePass(new nodes::DepthPrepass());
	std::shared_ptr<nodes::DepthReduction> depthReduction(new nodes::DepthReduction());
	std::shared_ptr<nodes::DepthReductionFinal> depthReductionFinal(new nodes::DepthReductionFinal());
	std::shared_ptr<nodes::CSM> csm(new nodes::CSM());
	std::shared_ptr<nodes::DrawSky> drawSky(new nodes::DrawSky());
	std::shared_ptr<nodes::DebugDraw> debugDraw(new nodes::DebugDraw());
	std::shared_ptr<nodes::LightCulling> lightCulling(new nodes::LightCulling());
	std::shared_ptr<nodes::BrightLumPass> brightLumPass(new nodes::BrightLumPass());
	std::shared_ptr<nodes::LuminanceReduction> luminanceReduction(new nodes::LuminanceReduction());
	std::shared_ptr<nodes::LuminanceReductionFinal> luminanceReductionFinal(new nodes::LuminanceReductionFinal());
	std::shared_ptr<nodes::HDRCombine> hdrCombine(new nodes::HDRCombine());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample2(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample4(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample8(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample16(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample32(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical32(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical16(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical8(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical4(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical2(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal32(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal16(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal8(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal4(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal2(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomAdd> bloomAdd3216(new nodes::BloomAdd());
	std::shared_ptr<nodes::BloomAdd> bloomAdd168(new nodes::BloomAdd());
	std::shared_ptr<nodes::BloomAdd> bloomAdd84(new nodes::BloomAdd());
	std::shared_ptr<nodes::BloomAdd> bloomAdd42(new nodes::BloomAdd());
	std::shared_ptr<nodes::TileMax> tileMax(new nodes::TileMax());
	std::shared_ptr<nodes::NeighborMax> neighborMax(new nodes::NeighborMax());
	std::shared_ptr<nodes::MotionBlur> motionBlur(new nodes::MotionBlur());
	std::shared_ptr<nodes::LensFlare> lensFlare(new nodes::LensFlare());
	std::shared_ptr<nodes::BloomBlur> lensFlareBlurHorizontal(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> lensFlareBlurVertical(new nodes::BloomBlur());
	auto smaaAreaEnv = std::make_shared<nodes::GetEnvVariable>();
	auto smaaSearchEnv = std::make_shared<nodes::GetEnvVariable>();
	auto lensFlareColorEnv = std::make_shared<nodes::GetEnvVariable>();
	auto colorGradingEnv = std::make_shared<nodes::GetEnvVariable>();
	auto lensFlareDirtEnv = std::make_shared<nodes::GetEnvVariable>();
	auto lensFlareStarEnv = std::make_shared<nodes::GetEnvVariable>();
	std::shared_ptr<nodes::SMAA> smaa(new nodes::SMAA());
	std::shared_ptr<nodes::DOFPrepare> dofPrepare(new nodes::DOFPrepare());
	std::shared_ptr<nodes::DOFTileMax> dofTileMax(new nodes::DOFTileMax());
	std::shared_ptr<nodes::DOFNeighborMax> dofNeighborMax(new nodes::DOFNeighborMax());
	std::shared_ptr<nodes::DOFMain> dofMain(new nodes::DOFMain());
	std::shared_ptr<nodes::Voxelization> voxelization(new nodes::Voxelization());
	std::shared_ptr<nodes::VolumetricLighting> volumetricLighting(new nodes::VolumetricLighting());
	std::shared_ptr<nodes::ShadowMapGen> shadowMapGen(new nodes::ShadowMapGen());
	std::shared_ptr<nodes::CreateTexture> createShadowmapTextures(new nodes::CreateTexture());
	std::shared_ptr<nodes::ScreenSpaceShadow> screenSpaceShadow(new nodes::ScreenSpaceShadow());
	std::shared_ptr<nodes::ScreenSpaceReflection> screenSpaceReflection(new nodes::ScreenSpaceReflection());
	std::shared_ptr<nodes::TextRender> textRender(new nodes::TextRender());
	auto fontTexEnv = std::make_shared<nodes::GetEnvVariable>();
	auto fontBinaryEnv = std::make_shared<nodes::GetEnvVariable>();
	std::shared_ptr<nodes::ScreenSpaceAmbientOcclusion> screenSpaceAmbientOcclusion(new nodes::ScreenSpaceAmbientOcclusion());
	TextureUsage usage;

	backBufferProperties->SetDisplayName("backBufferProperties");
	createDepthBuffer->SetDisplayName("createDepthBuffer");
	createHdrRenderTarget->SetDisplayName("createHdrRenderTarget");
	forwardRender->SetDisplayName("forwardRender");
	depthPrePass->SetDisplayName("depthPrePass");
	depthReduction->SetDisplayName("depthReduction");
	depthReductionFinal->SetDisplayName("depthReductionFinal");
	csm->SetDisplayName("csm");
	drawSky->SetDisplayName("drawSky");
	debugDraw->SetDisplayName("debugDraw");
	lightCulling->SetDisplayName("lightCulling");
	brightLumPass->SetDisplayName("brightLumPass");
	luminanceReduction->SetDisplayName("luminanceReduction");
	luminanceReductionFinal->SetDisplayName("luminanceReductionFinal");
	hdrCombine->SetDisplayName("hdrCombine");
	bloomDownsample2->SetDisplayName("bloomDownsample2");
	bloomDownsample4->SetDisplayName("bloomDownsample4");
	bloomDownsample8->SetDisplayName("bloomDownsample8");
	bloomDownsample16->SetDisplayName("bloomDownsample16");
	bloomDownsample32->SetDisplayName("bloomDownsample32");
	bloomBlurVertical32->SetDisplayName("bloomBlurVertical32");
	bloomBlurVertical16->SetDisplayName("bloomBlurVertical16");
	bloomBlurVertical8->SetDisplayName("bloomBlurVertical8");
	bloomBlurVertical4->SetDisplayName("bloomBlurVertical4");
	bloomBlurVertical2->SetDisplayName("bloomBlurVertical2");
	bloomBlurHorizontal32->SetDisplayName("bloomBlurHorizontal32");
	bloomBlurHorizontal16->SetDisplayName("bloomBlurHorizontal16");
	bloomBlurHorizontal8->SetDisplayName("bloomBlurHorizontal8");
	bloomBlurHorizontal4->SetDisplayName("bloomBlurHorizontal4");
	bloomBlurHorizontal2->SetDisplayName("bloomBlurHorizontal2");
	bloomAdd3216->SetDisplayName("bloomAdd3216");
	bloomAdd168->SetDisplayName("bloomAdd168");
	bloomAdd84->SetDisplayName("bloomAdd84");
	bloomAdd42->SetDisplayName("bloomAdd42");
	tileMax->SetDisplayName("tileMax");
	neighborMax->SetDisplayName("neighborMax");
	motionBlur->SetDisplayName("motionBlur");
	lensFlare->SetDisplayName("lensFlare");
	lensFlareBlurHorizontal->SetDisplayName("lensFlareBlurHorizontal");
	lensFlareBlurVertical->SetDisplayName("lensFlareBlurVertical");
	smaaAreaEnv->SetDisplayName("smaaAreaEnv");
	smaaSearchEnv->SetDisplayName("smaaSearchEnv");
	lensFlareColorEnv->SetDisplayName("lensFlareColorEnv");
	colorGradingEnv->SetDisplayName("colorGradingEnv");
	lensFlareDirtEnv->SetDisplayName("lensFlareDirtEnv");
	lensFlareStarEnv->SetDisplayName("lensFlareStarEnv");
	smaa->SetDisplayName("smaa");
	dofPrepare->SetDisplayName("dofPrepare");
	dofTileMax->SetDisplayName("dofTileMax");
	dofNeighborMax->SetDisplayName("dofNeighborMax");
	dofMain->SetDisplayName("dofMain");
	voxelization->SetDisplayName("voxelization");
	volumetricLighting->SetDisplayName("volumetricLighting");
	shadowMapGen->SetDisplayName("shadowMapGen");
	createShadowmapTextures->SetDisplayName("createShadowmapTextures");
	screenSpaceShadow->SetDisplayName("screenSpaceShadow");
	screenSpaceReflection->SetDisplayName("screenSpaceReflection");
	textRender->SetDisplayName("textRender");
	fontTexEnv->SetDisplayName("fontTexEnv");
	fontBinaryEnv->SetDisplayName("fontBinaryEnv");
	screenSpaceAmbientOcclusion->SetDisplayName("screenSpaceAmbientOcclusion");


	backBufferProperties->GetInput<0>().Link(getBackBuffer->GetOutput(0));

	createDepthBuffer->GetInput<0>().Link(backBufferProperties->GetOutput(0));
	createDepthBuffer->GetInput<1>().Link(backBufferProperties->GetOutput(1));
	createDepthBuffer->GetInput<2>().Set(gxapi::eFormat::R32G8X24_TYPELESS);
	createDepthBuffer->GetInput<3>().Set(1);
	usage = TextureUsage();
	usage.depthStencil = true;
	createDepthBuffer->GetInput<4>().Set(usage);

	depthPrePass->GetInput(0)->Link(createDepthBuffer->GetOutput(0));
	depthPrePass->GetInput(1)->Link(getWorldScene->GetOutput(0));
	depthPrePass->GetInput(2)->Link(getCamera->GetOutput(0));

	depthReduction->GetInput<0>().Link(depthPrePass->GetOutput(0));

	depthReductionFinal->GetInput<0>().Link(depthReduction->GetOutput(0));
	depthReductionFinal->GetInput<1>().Link(getCamera->GetOutput(0));
	depthReductionFinal->GetInput<2>().Link(getWorldScene->GetOutput(2));

	constexpr unsigned cascadeSize = 1024;
	constexpr unsigned numCascades = 4;

	createCsmTextures->GetInput<0>().Set(cascadeSize);
	createCsmTextures->GetInput<1>().Set(cascadeSize);
	createCsmTextures->GetInput<2>().Set(gxapi::eFormat::R32_TYPELESS);
	createCsmTextures->GetInput<3>().Set(numCascades);
	usage = TextureUsage();
	usage.depthStencil = true;
	createCsmTextures->GetInput<4>().Set(usage);
	createCsmTextures->GetInput<5>().Set(false);

	csm->GetInput<0>().Link(createCsmTextures->GetOutput(0));
	csm->GetInput<1>().Link(getWorldScene->GetOutput(0));
	csm->GetInput<2>().Link(depthReductionFinal->GetOutput(0));

	createShadowmapTextures->GetInput<0>().Set(1024);
	createShadowmapTextures->GetInput<1>().Set(1024);
	createShadowmapTextures->GetInput<2>().Set(gxapi::eFormat::R32_TYPELESS);
	createShadowmapTextures->GetInput<3>().Set(1);
	createShadowmapTextures->GetInput<4>().Set(usage);
	createShadowmapTextures->GetInput<5>().Set(true);

	shadowMapGen->GetInput(0)->Link(createShadowmapTextures->GetOutput(0));
	shadowMapGen->GetInput(1)->Link(getWorldScene->GetOutput(0));

	screenSpaceShadow->GetInput(0)->Link(depthPrePass->GetOutput(0));
	screenSpaceShadow->GetInput(1)->Link(getCamera->GetOutput(0));

	//TODO (2.5D light culling + verify)
	lightCulling->GetInput<0>().Link(depthPrePass->GetOutput(0));
	lightCulling->GetInput<1>().Link(getCamera->GetOutput(0));

	createHdrRenderTarget->GetInput<0>().Link(backBufferProperties->GetOutput(0));
	createHdrRenderTarget->GetInput<1>().Link(backBufferProperties->GetOutput(1));
	createHdrRenderTarget->GetInput<2>().Set(gxapi::eFormat::R16G16B16A16_FLOAT);
	createHdrRenderTarget->GetInput<3>().Set(1);
	usage = TextureUsage();
	usage.renderTarget = true;
	createHdrRenderTarget->GetInput<4>().Set(usage);
	createHdrRenderTarget->GetInput<6>().Set(true);

	forwardRender->GetInput(0)->Link(createHdrRenderTarget->GetOutput(0));
	forwardRender->GetInput(1)->Link(depthPrePass->GetOutput(0));
	forwardRender->GetInput(2)->Link(getWorldScene->GetOutput(0));
	forwardRender->GetInput(3)->Link(getCamera->GetOutput(0));
	forwardRender->GetInput(4)->Link(getWorldScene->GetOutput(2));
	forwardRender->GetInput(5)->Link(csm->GetOutput(0));
	forwardRender->GetInput(6)->Link(depthReductionFinal->GetOutput(1));
	forwardRender->GetInput(7)->Link(depthReductionFinal->GetOutput(2));
	forwardRender->GetInput(8)->Link(depthReductionFinal->GetOutput(0));
	forwardRender->GetInput(9)->Link(lightCulling->GetOutput(0));

	screenSpaceAmbientOcclusion->GetInput(0)->Link(depthPrePass->GetOutput(0));
	screenSpaceAmbientOcclusion->GetInput(1)->Link(getCamera->GetOutput(0));

	volumetricLighting->GetInput(0)->Link(depthPrePass->GetOutput(0));
	volumetricLighting->GetInput(1)->Link(forwardRender->GetOutput(0));
	volumetricLighting->GetInput(2)->Link(lightCulling->GetOutput(0));
	volumetricLighting->GetInput(3)->Link(getCamera->GetOutput(0));
	volumetricLighting->GetInput(4)->Link(csm->GetOutput(0));
	volumetricLighting->GetInput(5)->Link(depthReductionFinal->GetOutput(1));
	volumetricLighting->GetInput(6)->Link(depthReductionFinal->GetOutput(2));

	voxelization->GetInput(0)->Link(getWorldScene->GetOutput(0));
	voxelization->GetInput(1)->Link(getCamera->GetOutput(0));
	voxelization->GetInput(2)->Link(forwardRender->GetOutput(0)); //only for visualization
	voxelization->GetInput(3)->Link(depthPrePass->GetOutput(0));
	voxelization->GetInput(4)->Link(csm->GetOutput(0));
	voxelization->GetInput(5)->Link(depthReductionFinal->GetOutput(3));

	drawSky->GetInput<0>().Link(forwardRender->GetOutput(0));
	drawSky->GetInput<1>().Link(depthPrePass->GetOutput(0));
	drawSky->GetInput<2>().Link(getCamera->GetOutput(0));
	drawSky->GetInput<3>().Link(getWorldScene->GetOutput(2));

	screenSpaceReflection->GetInput(0)->Link(drawSky->GetOutput(0));
	screenSpaceReflection->GetInput(1)->Link(depthPrePass->GetOutput(0));
	screenSpaceReflection->GetInput(2)->Link(getCamera->GetOutput(0));

	tileMax->GetInput<0>().Link(forwardRender->GetOutput(1));
	
	neighborMax->GetInput<0>().Link(tileMax->GetOutput(0));

	motionBlur->GetInput<0>().Link(drawSky->GetOutput(0));
	motionBlur->GetInput<1>().Link(forwardRender->GetOutput(1));
	motionBlur->GetInput<2>().Link(neighborMax->GetOutput(0));
	motionBlur->GetInput<3>().Link(depthPrePass->GetOutput(0));

	dofPrepare->GetInput<0>().Link(motionBlur->GetOutput(0));
	dofPrepare->GetInput<1>().Link(depthPrePass->GetOutput(0));
	dofPrepare->GetInput<2>().Link(getCamera->GetOutput(0));

	dofTileMax->GetInput<0>().Link(dofPrepare->GetOutput(0));
	//dofTileMax->GetInput<1>().Link(depthPrePass->GetOutput(0));
	dofTileMax->GetInput<1>().Link(dofPrepare->GetOutput(1));

	dofNeighborMax->GetInput<0>().Link(dofTileMax->GetOutput(0));

	dofMain->GetInput<0>().Link(dofPrepare->GetOutput(0));
	dofMain->GetInput<1>().Link(dofPrepare->GetOutput(1));
	dofMain->GetInput<2>().Link(dofNeighborMax->GetOutput(0));
	dofMain->GetInput<3>().Link(getCamera->GetOutput(0));
	dofMain->GetInput<4>().Link(motionBlur->GetOutput(0));
	dofMain->GetInput<5>().Link(depthPrePass->GetOutput(0));

	brightLumPass->GetInput<0>().Link(motionBlur->GetOutput(0));
	
	bloomDownsample2->GetInput<0>().Link(brightLumPass->GetOutput(0));
	bloomDownsample4->GetInput<0>().Link(bloomDownsample2->GetOutput(0));
	bloomDownsample8->GetInput<0>().Link(bloomDownsample4->GetOutput(0));
	bloomDownsample16->GetInput<0>().Link(bloomDownsample8->GetOutput(0));
	bloomDownsample32->GetInput<0>().Link(bloomDownsample16->GetOutput(0));

	bloomBlurVertical32->GetInput<0>().Link(bloomDownsample32->GetOutput(0));
	bloomBlurVertical32->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal32->GetInput<0>().Link(bloomBlurVertical32->GetOutput(0));
	bloomBlurHorizontal32->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd3216->GetInput<0>().Link(bloomBlurHorizontal32->GetOutput(0));
	bloomAdd3216->GetInput<1>().Link(bloomDownsample16->GetOutput(0));

	bloomBlurVertical16->GetInput<0>().Link(bloomAdd3216->GetOutput(0));
	bloomBlurVertical16->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal16->GetInput<0>().Link(bloomBlurVertical16->GetOutput(0));
	bloomBlurHorizontal16->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd168->GetInput<0>().Link(bloomBlurHorizontal16->GetOutput(0));
	bloomAdd168->GetInput<1>().Link(bloomDownsample8->GetOutput(0));

	bloomBlurVertical8->GetInput<0>().Link(bloomAdd168->GetOutput(0));
	bloomBlurVertical8->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal8->GetInput<0>().Link(bloomBlurVertical8->GetOutput(0));
	bloomBlurHorizontal8->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd84->GetInput<0>().Link(bloomBlurHorizontal8->GetOutput(0));
	bloomAdd84->GetInput<1>().Link(bloomDownsample4->GetOutput(0));

	bloomBlurVertical4->GetInput<0>().Link(bloomAdd84->GetOutput(0));
	bloomBlurVertical4->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal4->GetInput<0>().Link(bloomBlurVertical4->GetOutput(0));
	bloomBlurHorizontal4->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd42->GetInput<0>().Link(bloomBlurHorizontal4->GetOutput(0));
	bloomAdd42->GetInput<1>().Link(bloomDownsample2->GetOutput(0));

	bloomBlurVertical2->GetInput<0>().Link(bloomAdd42->GetOutput(0));
	bloomBlurVertical2->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal2->GetInput<0>().Link(bloomBlurVertical2->GetOutput(0));
	bloomBlurHorizontal2->GetInput<1>().Set(Vec2(1, 0));

	lensFlareColorEnv->GetInput<0>().Set("LensFlare_ColorTex");
	lensFlare->GetInput<0>().Link(bloomDownsample4->GetOutput(0));
	lensFlare->GetInput<1>().Link(lensFlareColorEnv->GetOutput(0));

	lensFlareBlurVertical->GetInput<0>().Link(lensFlare->GetOutput(0));
	lensFlareBlurVertical->GetInput<1>().Set(Vec2(0, 1));
	lensFlareBlurHorizontal->GetInput<0>().Link(lensFlareBlurVertical->GetOutput(0));
	lensFlareBlurHorizontal->GetInput<1>().Set(Vec2(1, 0));

	luminanceReduction->GetInput<0>().Link(brightLumPass->GetOutput(1));

	luminanceReductionFinal->GetInput<0>().Link(luminanceReduction->GetOutput(0));

	colorGradingEnv->GetInput<0>().Set("HDRCombine_colorGradingTex");
	lensFlareDirtEnv->GetInput<0>().Set("HDRCombine_lensFlareDirtTex");
	lensFlareStarEnv->GetInput<0>().Set("HDRCombine_lensFlareStarTex");
	//hdrCombine->GetInput<0>().Link(motionBlur->GetOutput(0));
	//hdrCombine->GetInput<0>().Link(dofMain->GetOutput(0));
	hdrCombine->GetInput<0>().Link(drawSky->GetOutput(0));
	hdrCombine->GetInput<1>().Link(luminanceReductionFinal->GetOutput(0));
	hdrCombine->GetInput<2>().Link(bloomBlurHorizontal2->GetOutput(0));
	hdrCombine->GetInput<3>().Link(lensFlareBlurHorizontal->GetOutput(0));
	hdrCombine->GetInput<4>().Link(colorGradingEnv->GetOutput(0));
	hdrCombine->GetInput<5>().Link(lensFlareDirtEnv->GetOutput(0));
	hdrCombine->GetInput<6>().Link(lensFlareStarEnv->GetOutput(0));
	hdrCombine->GetInput<7>().Link(getCamera->GetOutput(0));

	// last step in world render is debug draw
	//debugDraw->GetInput<0>().Link(drawSky->GetOutput(0));
	//debugDraw->GetInput<0>().Link(forwardRender->GetOutput(1));
	debugDraw->GetInput<0>().Link(hdrCombine->GetOutput(0));
	debugDraw->GetInput<1>().Link(getCamera->GetOutput(0));

	smaaAreaEnv->GetInput<0>().Set("SMAA_areaTex");
	smaaSearchEnv->GetInput<0>().Set("SMAA_searchTex");
	smaa->GetInput<0>().Link(debugDraw->GetOutput(0));
	smaa->GetInput(1)->Link(smaaAreaEnv->GetOutput(0));
	smaa->GetInput<2>().Link(smaaSearchEnv->GetOutput(0));

	fontTexEnv->GetInput<0>().Set("TextRender_fontTex");
	fontBinaryEnv->GetInput<0>().Set("TextRender_fontBinary");
	textRender->GetInput<0>().Link(smaa->GetOutput(0));
	textRender->GetInput<1>().Link(fontTexEnv->GetOutput(0));
	textRender->GetInput<2>().Link(fontBinaryEnv->GetOutput(0));


	// -----------------------------
	// Gui pipeline path
	// -----------------------------
	std::shared_ptr<nodes::GetSceneByName> getGuiScene(new nodes::GetSceneByName());
	std::shared_ptr<nodes::GetCameraByName> getGuiCamera(new nodes::GetCameraByName());
	std::shared_ptr<nodes::OverlayRender> guiRender(new nodes::OverlayRender());
	std::shared_ptr<nodes::BlendWithTransform> alphaBlend(new nodes::BlendWithTransform());
	std::shared_ptr<nodes::ScreenSpaceTransform> createWorldRenderTransform(new nodes::ScreenSpaceTransform());

	getGuiScene->GetInput<0>().Set("Gui");
	getGuiCamera->GetInput<0>().Set("GuiCamera");

	createWorldRenderTransform->GetInput<0>().Link(backBufferProperties->GetOutput(0));
	createWorldRenderTransform->GetInput<1>().Link(backBufferProperties->GetOutput(1));
	createWorldRenderTransform->GetInput<2>().Link(getWorldRenderPos->GetOutput(0));
	createWorldRenderTransform->GetInput<3>().Link(getWorldRenderRot->GetOutput(0));
	createWorldRenderTransform->GetInput<4>().Link(getWorldRenderSize->GetOutput(0));

	//createWorldRenderTransform->GetInput<0>().Set(800);
	//createWorldRenderTransform->GetInput<1>().Set(600);
	//createWorldRenderTransform->GetInput<2>().Set(Vec2(0.f, 0.f));
	//createWorldRenderTransform->GetInput<3>().Set(0);
	//createWorldRenderTransform->GetInput<4>().Set(Vec2(800.f, 600.f));

	guiRender->GetInput<0>().Link(getBackBuffer->GetOutput(0));
	guiRender->GetInput<1>().Link(getGuiScene->GetOutput(1));
	guiRender->GetInput<2>().Link(getGuiCamera->GetOutput(0));

	// NOTE: the intended behaviour of this blending is to draw the render target on top of the shader output (render target contains overlay, shader output contains scene)
	gxapi::RenderTargetBlendState blending;
	blending.enableBlending = true;
	blending.alphaOperation = gxapi::eBlendOperation::ADD;
	blending.shaderAlphaFactor = gxapi::eBlendOperand::INV_TARGET_ALPHA;
	blending.targetAlphaFactor = gxapi::eBlendOperand::TARGET_ALPHA;
	blending.colorOperation = gxapi::eBlendOperation::ADD;
	blending.shaderColorFactor = gxapi::eBlendOperand::INV_TARGET_ALPHA;
	blending.targetColorFactor = gxapi::eBlendOperand::TARGET_ALPHA;
	blending.enableLogicOp = false;
	blending.mask = gxapi::eColorMask::ALL;

	alphaBlend->GetInput<0>().Link(guiRender->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(debugDraw->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(smaa->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(voxelization->GetOutput(1));
	//alphaBlend->GetInput<1>().Link(volumetricLighting->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(screenSpaceShadow->GetOutput(0));
	alphaBlend->GetInput<1>().Link(screenSpaceReflection->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(screenSpaceAmbientOcclusion->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(textRender->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(dofMain->GetOutput(0));
	alphaBlend->GetInput<2>().Set(blending);
	//alphaBlend->GetInput<3>().Set(Mat44::FromScaleVector(Vec3(.5f, 1.f, 1.f)));
	alphaBlend->GetInput<3>().Link(createWorldRenderTransform->GetOutput(0));

	m_graphicsNodes = {
		getWorldScene,
		getCamera,
		getBackBuffer,

		getWorldRenderPos,
		getWorldRenderRot,
		getWorldRenderSize,
		worldRenderSizeSplit,

		backBufferProperties,
		createDepthBuffer,
		createHdrRenderTarget,
		createCsmTextures,
		forwardRender,
		depthPrePass,
		depthReduction,
		depthReductionFinal,
		csm,
		drawSky,
		lightCulling,
		brightLumPass,
		luminanceReduction,
		luminanceReductionFinal,
		bloomDownsample2,
		bloomDownsample4,
		bloomDownsample8,
		bloomDownsample16,
		bloomDownsample32,
		bloomBlurVertical32,
		bloomBlurHorizontal32,
		bloomAdd3216,
		bloomBlurVertical16,
		bloomBlurHorizontal16,
		bloomAdd168,
		bloomBlurVertical8,
		bloomBlurHorizontal8,
		bloomAdd84,
		bloomBlurVertical4,
		bloomBlurHorizontal4,
		bloomAdd42,
		bloomBlurVertical2,
		bloomBlurHorizontal2,
		hdrCombine,
		tileMax,
		neighborMax,
		motionBlur,
		lensFlare,
		lensFlareBlurHorizontal,
		lensFlareBlurVertical,
		smaa,
		dofPrepare,
		dofTileMax,
		dofNeighborMax,
		smaaAreaEnv,
		smaaSearchEnv,
		lensFlareColorEnv,
		colorGradingEnv,
		lensFlareDirtEnv,
		lensFlareStarEnv,
		dofMain,
		voxelization,
		volumetricLighting,
		//shadowMapGen,
		screenSpaceShadow,
		screenSpaceReflection,
		textRender,
		fontTexEnv,
		fontBinaryEnv,
		screenSpaceAmbientOcclusion,

		getGuiScene,
		getGuiCamera,
		guiRender,
		//alphaBlend,
		createWorldRenderTransform,
		debugDraw,
		alphaBlend
	};


	std::vector<std::shared_ptr<NodeBase>> nodeList;
	nodeList.reserve(m_graphicsNodes.size());
	for (auto curr : m_graphicsNodes) {
		nodeList.push_back(curr);
	}

	EngineContext engineContext(1, 1);
	for (auto& node : nodeList) {
		if (auto graphicsNode = dynamic_cast<GraphicsNode*>(node.get())) {
			graphicsNode->Initialize(engineContext);
		}
	}

	m_pipeline.CreateFromNodesList(nodeList);

	DumpPipelineGraph(m_pipeline, "pipeline_graph.dot");

	m_specialNodes = SelectSpecialNodes(m_pipeline);
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

	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	Texture2D backBuffer = m_backBufferHeap->GetBackBuffer(backBufferIndex).GetResource();

	for (auto node : m_specialNodes) {
		if (auto* getScene = dynamic_cast<nodes::GetSceneByName*>(node)) {
			getScene->SetSceneList(scenes);
		}
		else if (auto* getCamera = dynamic_cast<nodes::GetCameraByName*>(node)) {
			getCamera->SetCameraList(cameras);
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
	m_nodeFactory.RegisterNodeClass<nodes::GetTime>("Pipeline/System");
	m_nodeFactory.RegisterNodeClass<nodes::GetEnvVariable>("Pipeline/System");

	m_nodeFactory.RegisterNodeClass<nodes::TextureProperties>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::CreateTexture>("Pipeline/Utility");
	m_nodeFactory.RegisterNodeClass<nodes::VectorComponents<1>>("Pipeline/Utility");

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
	m_nodeFactory.RegisterNodeClass<nodes::TextRender>("Pipeline/Render");
	m_nodeFactory.RegisterNodeClass<nodes::ScreenSpaceAmbientOcclusion>("Pipeline/Render");

	m_nodeFactory.RegisterNodeClass<nodes::OverlayRender>("Pipeline/Render");
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
	std::map<const NodeBase*, int> nodeIndexMap;

	// Fill node map and parent maps
	for (const NodeBase& node : pipeline) {
		int nodeIndex = nodeIndexMap.size();
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