#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging

#include "Nodes/Node_FrameCounter.hpp"
#include "Nodes/Node_FrameColor.hpp"
#include "Nodes/Node_GetBackBuffer.hpp"
#include "Nodes/Node_GetSceneByName.hpp"
#include "Nodes/Node_GetCameraByName.hpp"
#include "Nodes/Node_GetTime.hpp"

//forward
#include "Nodes/Node_ForwardRender.hpp"
#include "Nodes/Node_DepthPrepass.hpp"
#include "Nodes/Node_DepthReduction.hpp"

#include "Nodes/Node_GenCSM.hpp"
#include "Nodes/Node_RenderToBackBuffer.hpp"
#include "Nodes/Node_DrawSky.hpp"

#include "WindowResizeListener.hpp"

#include "Scene.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Image.hpp"
#include "MeshEntity.hpp"


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
	m_shaderManager.AddSourceDirectory("./Shaders");
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

	for (auto curr : m_windowResizeListeners) {
		curr->WindowResized(width, height);
	}
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

Camera* GraphicsEngine::CreateCamera(std::string name) {
	class ObservedCamera : public Camera {
	public:
		ObservedCamera(std::function<void(Camera*)> deleteHandler, std::string name) :
			m_deleteHandler(std::move(deleteHandler))
		{
			SetName(name);
		}
		~ObservedCamera() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<Camera*>(this)); }
		}
	protected:
		std::function<void(Camera*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterCamera = [this](Camera* arg) {
		m_cameras.erase(arg);
	};

	// Allocate a new scene, and register it.
	Camera* camera = new ObservedCamera(unregisterCamera, std::move(name));
	m_cameras.insert(camera);

	return camera;
}

MeshEntity* GraphicsEngine::CreateMeshEntity() {
	return new MeshEntity;
}


void GraphicsEngine::CreatePipeline() {
	auto swapChainDesc = m_swapChain->GetDesc();

	std::unique_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::unique_ptr<nodes::GetCameraByName> getCamera(new nodes::GetCameraByName());
	std::unique_ptr<nodes::RenderToBackBuffer> renderToBackbuffer(new nodes::RenderToBackBuffer(m_graphicsApi));

	std::unique_ptr<nodes::ForwardRender> forwardRender(new nodes::ForwardRender(m_graphicsApi, swapChainDesc.width, swapChainDesc.height));
	std::unique_ptr<nodes::DepthPrepass> depthPrePass(new nodes::DepthPrepass(m_graphicsApi, swapChainDesc.width, swapChainDesc.height));
	std::unique_ptr<nodes::DepthReduction> depthReduction(new nodes::DepthReduction(m_graphicsApi, swapChainDesc.width, swapChainDesc.height));

	getWorldScene->GetInput<0>().Set("World");
	getCamera->GetInput<0>().Set("WorldCam");

	depthPrePass->GetInput<0>().Link(getWorldScene->GetOutput(0));
	depthPrePass->GetInput<1>().Link(getCamera->GetOutput(0));

	depthReduction->GetInput<0>().Link(depthPrePass->GetOutput(0));

	forwardRender->GetInput<0>().Link(depthReduction->GetOutput(1));
	forwardRender->GetInput<1>().Link(getWorldScene->GetOutput(0));
	forwardRender->GetInput<2>().Link(getCamera->GetOutput(0));
	forwardRender->GetInput<3>().Link(getWorldScene->GetOutput(1));

	//renderToBackbuffer->GetInput<0>().Link(forwardRender->GetOutput(0));
	renderToBackbuffer->GetInput<0>().Link(depthReduction->GetOutput(0));

	GraphicsContext graphicsContext(&m_memoryManager, &m_persResViewHeap, &m_rtvHeap, &m_dsvHeap, std::thread::hardware_concurrency(), 1, &m_shaderManager, m_graphicsApi);

	getWorldScene->InitGraphics(graphicsContext);
	getCamera->InitGraphics(graphicsContext);
	depthPrePass->InitGraphics(graphicsContext);
	depthReduction->InitGraphics(graphicsContext);
	forwardRender->InitGraphics(graphicsContext);
	renderToBackbuffer->InitGraphics(graphicsContext);

	m_windowResizeListeners.push_back(forwardRender.get());
	m_windowResizeListeners.push_back(depthPrePass.get());
	m_windowResizeListeners.push_back(depthReduction.get());

	{
		m_pipeline.CreateFromNodesList(
			{
				getWorldScene.get(),
				getCamera.get(),
				depthPrePass.get(),
				depthReduction.get(),
				forwardRender.get(),
				renderToBackbuffer.get()
			},
			std::default_delete<exc::NodeBase>()
		);

		getWorldScene.release();
		getCamera.release();
		depthPrePass.release();
		depthReduction.release();
		forwardRender.release();
		renderToBackbuffer.release();
	}
}


} // namespace gxeng
} // namespace inl