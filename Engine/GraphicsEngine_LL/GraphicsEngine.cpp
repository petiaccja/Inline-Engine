#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging

#include "Nodes/Node_GetBackBuffer.hpp"
#include "Nodes/Node_GetSceneByName.hpp"
#include "Nodes/Node_GetCameraByName.hpp"
#include "Nodes/Node_GetTime.hpp"

//forward
#include "Nodes/Node_ForwardRender.hpp"
#include "Nodes/Node_DepthPrepass.hpp"
#include "Nodes/Node_DepthReduction.hpp"
#include "Nodes/Node_DepthReductionFinal.hpp"
#include "Nodes/Node_CSM.hpp"
#include "Nodes/Node_DrawSky.hpp"

//Gui
#include "Nodes/Node_OverlayRender.hpp"
#include "Nodes/Node_Blend.hpp"

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
#ifdef NDEBUG
	m_shaderManager.SetShaderCompileFlags(gxapi::eShaderCompileFlags::OPTIMIZATION_HIGH);
#else
	gxapi::eShaderCompileFlags flags = gxapi::eShaderCompileFlags::NO_OPTIMIZATION;
	flags += gxapi::eShaderCompileFlags::DEBUG;
	m_shaderManager.SetShaderCompileFlags(flags);
#endif // NDEBUG


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
}


GraphicsEngine::~GraphicsEngine() {
	SyncPoint lastSync = m_masterCommandQueue.Signal();
	lastSync.Wait();
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

	std::vector<UploadManager::UploadDescription> uploadRequests = m_memoryManager.GetUploadManager()._TakeQueuedUploads();
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
}


void GraphicsEngine::SetScreenSize(unsigned width, unsigned height) {
	if (width == 0 || height == 0) {
		return;
	}

	SyncPoint sp = m_masterCommandQueue.Signal();
	sp.Wait();

	m_backBufferHeap.reset();
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


void GraphicsEngine::CreatePipeline() {
	auto swapChainDesc = m_swapChain->GetDesc();

	// -----------------------------
	// 3D pipeline path
	// -----------------------------
	std::shared_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::shared_ptr<nodes::GetCameraByName> getCamera(new nodes::GetCameraByName());
	std::shared_ptr<nodes::GetBackBuffer> getBackBuffer(new nodes::GetBackBuffer());

	std::shared_ptr<nodes::ForwardRender> forwardRender(new nodes::ForwardRender(m_graphicsApi));
	std::shared_ptr<nodes::DepthPrepass> depthPrePass(new nodes::DepthPrepass(m_graphicsApi));
	std::shared_ptr<nodes::DepthReduction> depthReduction(new nodes::DepthReduction(m_graphicsApi));
	std::shared_ptr<nodes::DepthReductionFinal> depthReductionFinal(new nodes::DepthReductionFinal(m_graphicsApi));
	std::shared_ptr<nodes::CSM> csm(new nodes::CSM(m_graphicsApi));
	std::shared_ptr<nodes::DrawSky> drawSky(new nodes::DrawSky(m_graphicsApi));


	getWorldScene->GetInput<0>().Set("World");
	getCamera->GetInput<0>().Set("WorldCam");

	static_assert(false, "Node graphics linking must be fixed. Do not even try to run, it's not gonna work.");

	// MISSING (fatal): link target depth tex to input(0) of depthPrePass
	depthPrePass->GetInput(1)->Link(getWorldScene->GetOutput(0));
	depthPrePass->GetInput(2)->Link(getCamera->GetOutput(0));

	depthReduction->GetInput<0>().Link(depthPrePass->GetOutput(0));

	depthReductionFinal->GetInput<0>().Link(depthReduction->GetOutput(0));
	depthReductionFinal->GetInput<1>().Link(getCamera->GetOutput(0));
	depthReductionFinal->GetInput<2>().Link(getWorldScene->GetOutput(1));

	csm->GetInput<0>().Link(getWorldScene->GetOutput(0));
	csm->GetInput<1>().Link(depthReductionFinal->GetOutput(0));

	// MISSING (fatal): link target color float16 tex to input(0) of forwardRender
	forwardRender->GetInput(1)->Link(depthPrePass->GetOutput(0));
	forwardRender->GetInput(2)->Link(getWorldScene->GetOutput(0));
	forwardRender->GetInput(3)->Link(getCamera->GetOutput(0));
	forwardRender->GetInput(4)->Link(getWorldScene->GetOutput(1));
	forwardRender->GetInput(5)->Link(csm->GetOutput(0));
	forwardRender->GetInput(6)->Link(depthReductionFinal->GetOutput(1));
	forwardRender->GetInput(7)->Link(depthReductionFinal->GetOutput(2));

	drawSky->GetInput<0>().Link(forwardRender->GetOutput(0));
	drawSky->GetInput<1>().Link(depthPrePass->GetOutput(0));
	drawSky->GetInput<2>().Link(getCamera->GetOutput(0));
	drawSky->GetInput<3>().Link(getWorldScene->GetOutput(1));


	// -----------------------------
	// Gui pipeline path
	// -----------------------------
	std::shared_ptr<nodes::GetSceneByName> getGuiScene(new nodes::GetSceneByName());
	std::shared_ptr<nodes::GetCameraByName> getGuiCamera(new nodes::GetCameraByName());
	std::shared_ptr<nodes::OverlayRender> guiRender(new nodes::OverlayRender(m_graphicsApi));
	std::shared_ptr<nodes::Blend> alphaBlend(new nodes::Blend(m_graphicsApi));

	getGuiScene->GetInput<0>().Set("Gui");
	getGuiCamera->GetInput<0>().Set("GuiCamera");

	guiRender->GetInput<0>().Link(getBackBuffer->GetOutput(0));
	guiRender->GetInput<1>().Link(getGuiScene->GetOutput(0));
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
	alphaBlend->GetInput<1>().Link(drawSky->GetOutput(0));
	alphaBlend->GetInput<2>().Set(blending);

	m_graphicsNodes = {
		getWorldScene,
		getCamera,
		depthPrePass,
		depthReduction,
		depthReductionFinal,
		csm,
		forwardRender,
		drawSky,

		getGuiScene,
		getGuiCamera,
		guiRender,
		alphaBlend
	};


	std::vector<std::shared_ptr<exc::NodeBase>> nodeList;
	nodeList.reserve(m_graphicsNodes.size());
	for (auto curr : m_graphicsNodes) {
		nodeList.push_back(curr);
	}
	m_pipeline.CreateFromNodesList(nodeList);


	EngineContext engineContext(1, 1);
	InitializeGraphicsNodes(m_pipeline, engineContext);
	m_specialNodes = SelectSpecialNodes(m_pipeline);
}


void GraphicsEngine::InitializeGraphicsNodes(Pipeline& pipeline, EngineContext& context) {
	for (Pipeline::NodeIterator it = pipeline.Begin(); it != pipeline.End(); ++it) {
		if (const GraphicsNode* ptr = dynamic_cast<const GraphicsNode*>(&*it)) {
			// Pipeline disallows linking of its nodes, that's why it only returns const pointers.
			// We are not changing linking configuration here, so const_cast is justified.
			const_cast<GraphicsNode*>(ptr)->Initialize(context);
		}
	}
}

std::vector<GraphicsNode*> GraphicsEngine::SelectSpecialNodes(Pipeline& pipeline) {
	std::vector<GraphicsNode*> specialNodes;

	for (Pipeline::NodeIterator it = pipeline.Begin(); it != pipeline.End(); ++it) {
		// Pipeline disallows linking of its nodes, that's why it only returns const pointers.
		// We are not changing linking configuration here, so const_cast is justified.
		if (const nodes::GetSceneByName* ptr = dynamic_cast<const nodes::GetSceneByName*>(&*it)) {
			specialNodes.push_back(const_cast<nodes::GetSceneByName*>(ptr));
		}
		else if (const nodes::GetCameraByName* ptr = dynamic_cast<const nodes::GetCameraByName*>(&*it)) {
			specialNodes.push_back(const_cast<nodes::GetCameraByName*>(ptr));
		}
		else if (const nodes::GetBackBuffer* ptr = dynamic_cast<const nodes::GetBackBuffer*>(&*it)) {
			specialNodes.push_back(const_cast<nodes::GetBackBuffer*>(ptr));
		}
		else if (const nodes::GetTime* ptr = dynamic_cast<const nodes::GetTime*>(&*it)) {
			specialNodes.push_back(const_cast<nodes::GetTime*>(ptr));
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
			getTime->SetTime(m_absoluteTime.count() / 1e9);
		}
	}
}


} // namespace gxeng
} // namespace inl