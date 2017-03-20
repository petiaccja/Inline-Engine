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

	context.commandQueue = &m_masterCommandQueue;
	context.backBuffer = &m_backBufferHeap->GetBackBuffer(backBufferIndex);
	context.scenes = &m_scenes;
	context.overlays = &m_overlays;
	context.cameras = &m_cameras;

	std::vector<UploadManager::UploadDescription> uploadRequests = m_memoryManager.GetUploadManager()._TakeQueuedUploads();
	context.uploadRequests = &uploadRequests;

	context.residencyQueue = &m_residencyQueue;

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

	InitializeGraphicsNodes();
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
	std::unique_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::unique_ptr<nodes::GetCameraByName> getCamera(new nodes::GetCameraByName());
	std::unique_ptr<nodes::GetBackBuffer> getBackBuffer(new nodes::GetBackBuffer());

	std::unique_ptr<nodes::ForwardRender> forwardRender(new nodes::ForwardRender(m_graphicsApi));
	std::unique_ptr<nodes::DepthPrepass> depthPrePass(new nodes::DepthPrepass(m_graphicsApi));
	std::unique_ptr<nodes::DepthReduction> depthReduction(new nodes::DepthReduction(m_graphicsApi));
	std::unique_ptr<nodes::DepthReductionFinal> depthReductionFinal(new nodes::DepthReductionFinal(m_graphicsApi));
	std::unique_ptr<nodes::CSM> csm(new nodes::CSM(m_graphicsApi));
	std::unique_ptr<nodes::DrawSky> drawSky(new nodes::DrawSky(m_graphicsApi));


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
	std::unique_ptr<nodes::GetSceneByName> getGuiScene(new nodes::GetSceneByName());
	std::unique_ptr<nodes::GetCameraByName> getGuiCamera(new nodes::GetCameraByName());
	std::unique_ptr<nodes::OverlayRender> guiRender(new nodes::OverlayRender(m_graphicsApi));
	std::unique_ptr<nodes::Blend> alphaBlend(new nodes::Blend(m_graphicsApi, nodes::Blend::CASUAL_ALPHA_BLEND));

	getGuiScene->GetInput<0>().Set("Gui");
	getGuiCamera->GetInput<0>().Set("GuiCamera");

	guiRender->GetInput<0>().Link(getGuiScene->GetOutput(0));
	guiRender->GetInput<1>().Link(getGuiCamera->GetOutput(0));

	alphaBlend->GetInput<0>().Link(getBackBuffer->GetOutput(0));
	alphaBlend->GetInput<1>().Link(drawSky->GetOutput(0));
	alphaBlend->GetInput<2>().Link(guiRender->GetOutput(0));

	m_graphicsNodes = {
		getWorldScene.release(),
		getCamera.release(),
		depthPrePass.release(),
		depthReduction.release(),
		depthReductionFinal.release(),
		csm.release(),
		forwardRender.release(),
		drawSky.release(),

		getGuiScene.release(),
		getGuiCamera.release(),
		guiRender.release(),
		alphaBlend.release()
	};

	std::vector<exc::NodeBase*> nodeList;
	try {
		InitializeGraphicsNodes();

		nodeList.reserve(m_graphicsNodes.size());
		for (auto curr : m_graphicsNodes) {
			nodeList.push_back(curr);
		}
	}
	catch (...) {
		for (auto currNode : m_graphicsNodes) {
			delete currNode;
		}
		throw;
	}
	// CreateFromNodesList frees up resources if anything goes wrong
	m_pipeline.CreateFromNodesList(
		nodeList,
		std::default_delete<exc::NodeBase>()
	);
}

void GraphicsEngine::InitializeGraphicsNodes() {
	GraphicsContext graphicsContext(&m_memoryManager, &m_persResViewHeap, &m_rtvHeap, &m_dsvHeap, std::thread::hardware_concurrency(), 1, &m_shaderManager, m_swapChain.get(), m_graphicsApi);
	for (auto curr : m_graphicsNodes) {
		curr->InitGraphics(graphicsContext);
	}
}


} // namespace gxeng
} // namespace inl