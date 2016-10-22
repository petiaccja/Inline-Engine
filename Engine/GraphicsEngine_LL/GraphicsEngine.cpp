#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging

#include "Nodes/Node_ClearRenderTarget.h"
#include "Nodes/Node_FrameCounter.hpp"
#include "Nodes/Node_FrameColor.hpp"
#include "Nodes/Node_GetBackBuffer.hpp"
#include "Nodes/Node_GetSceneByName.hpp"
#include "Nodes/Node_TescoRender.hpp"

#include "Scene.hpp"
#include "Mesh.hpp"
#include "MeshEntity.hpp"



namespace inl {
namespace gxeng {

using namespace gxapi;



GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc)
	: m_gxapiManager(desc.gxapiManager),
	m_graphicsApi(desc.graphicsApi),
	m_commandAllocatorPool(desc.graphicsApi),
	m_scratchSpacePool(desc.graphicsApi),
	m_masterCommandQueue(desc.graphicsApi->CreateCommandQueue(CommandQueueDesc{ eCommandListType::GRAPHICS }), desc.graphicsApi->CreateFence(0)),
	m_residencyQueue(std::unique_ptr<gxapi::IFence>(desc.graphicsApi->CreateFence(0))),
	m_descriptorHeap(desc.graphicsApi),
	m_memoryManager(desc.graphicsApi, &m_descriptorHeap),
	m_logger(desc.logger)
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
	m_backBufferHeap = std::make_unique<BackBufferHeap>(m_graphicsApi, m_swapChain.get());

	// Do more stuff...
	CreatePipeline();
	m_scheduler.SetPipeline(std::move(m_pipeline));

	// Init logger
	m_logStreamGeneral = m_logger->CreateLogStream("General");
	m_logStreamPipeline = m_logger->CreateLogStream("Pipeline");

	// Init misc stuff
	m_absoluteTime = decltype(m_absoluteTime)(0);
	m_commandAllocatorPool.SetLogStream(&m_logStreamPipeline);

	// DELETE THIS
	m_pipelineEventPrinter.SetLog(&m_logStreamPipeline);
	m_pipelineEventDispatcher += &m_pipelineEventPrinter;

}


GraphicsEngine::~GraphicsEngine() {
	// empty
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
	context.uploadRequests = &m_memoryManager.GetUploadHeap()._GetQueuedUploads();

	context.residencyQueue = &m_residencyQueue;

	// Execute the pipeline
	m_pipelineEventDispatcher.DispatchFrameBegin(m_frame);
	m_scheduler.Execute(context);
	m_pipelineEventDispatcher.DispatchFrameEnd(m_frame);

	// Mark frame completion
	SyncPoint frameEnd = m_masterCommandQueue.Signal();
	m_frameEndFenceValues[backBufferIndex] = frameEnd;
	m_pipelineEventDispatcher.DispatchDeviceFrameEnd(frameEnd, m_frame);

	// Clean up after frame
	// m_memoryManager.GetUploadHeap()._ClearQueuedUploads();

	// Flush log
	m_logger->Flush();

	// Present frame
	m_swapChain->Present();
	++m_frame;
}




// Resources
Mesh* GraphicsEngine::CreateMesh() {
	return new Mesh(&m_memoryManager);
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

MeshEntity* GraphicsEngine::CreateMeshEntity() {
	return new MeshEntity;
}


void GraphicsEngine::CreatePipeline() {
	std::unique_ptr<nodes::FrameCounter> frameCounter(new nodes::FrameCounter());
	std::unique_ptr<nodes::FrameColor> frameColor(new nodes::FrameColor());
	std::unique_ptr<nodes::GetBackBuffer> getBackBuffer(new nodes::GetBackBuffer());
	std::unique_ptr<nodes::ClearRenderTarget> clearRtv(new nodes::ClearRenderTarget());
	std::unique_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::unique_ptr<nodes::TescoRender> renderWorld(new nodes::TescoRender(m_graphicsApi, m_gxapiManager));

	getWorldScene->GetInput<0>().Set("World");

	// link frame clear path
	frameCounter->GetOutput(0)->Link(frameColor->GetInput(0));
	frameColor->GetOutput(0)->Link(clearRtv->GetInput(1));
	getBackBuffer->GetOutput(0)->Link(clearRtv->GetInput(0));
	// link renderscene path
	getWorldScene->GetOutput(0)->Link(renderWorld->GetInput(1));
	// link to pathes together
	clearRtv->GetOutput(0)->Link(renderWorld->GetInput(0));


	m_pipeline.CreateFromNodesList({ frameCounter.get(), frameColor.get(), getBackBuffer.get(), clearRtv.get(), getWorldScene.get(), renderWorld.get() }, std::default_delete<exc::NodeBase>());


	frameCounter.release();
	frameColor.release();
	getBackBuffer.release();
	clearRtv.release();
	getWorldScene.release();
	renderWorld.release();
}


} // namespace gxeng
} // namespace inl