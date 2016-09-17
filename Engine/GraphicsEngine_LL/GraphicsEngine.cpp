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
#include "TypedMesh.hpp"
#include "MeshEntity.hpp"



namespace inl {
namespace gxeng {

using namespace gxapi;



GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc)
	: m_commandAllocatorPool(desc.graphicsApi),
	m_scratchSpacePool(desc.graphicsApi),
	m_gxapiManager(desc.gxapiManager),
	m_graphicsApi(desc.graphicsApi),
	m_masterCommandQueue(desc.graphicsApi->CreateCommandQueue(CommandQueueDesc{ eCommandListType::GRAPHICS }), desc.graphicsApi->CreateFence(0)),
	m_scheduler(m_graphicsApi)
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

	m_frameEndFenceValues.resize(m_swapChain->GetDesc().numBuffers, {nullptr, 0});

	// Init backbuffer heap
	m_backBufferHeap = std::make_unique<BackBufferHeap>(m_graphicsApi, m_swapChain.get());

	// Do more stuff...
	CreatePipeline();
	m_scheduler.SetPipeline(std::move(m_pipeline));

	// Launch init and clean tasks
	m_runInitCleanTasks = true;
	m_initTaskThread = std::thread(std::bind(&GraphicsEngine::InitTaskThreadFunc, this));
	m_cleanTaskThread = std::thread(std::bind(&GraphicsEngine::CleanTaskThreadFunc, this));

	// Init logger
	m_logStreamGeneral = m_logger.CreateLogStream("General");
	m_logStreamPipeline = m_logger.CreateLogStream("Pipeline");

	m_logger.OpenStream(&std::cout);

	// Init misc stuff
	m_absoluteTime = decltype(m_absoluteTime)(0);
	//m_commandAllocatorPool.SetLogStream(&m_logStreamPipeline);
}


GraphicsEngine::~GraphicsEngine() {
	// Stop tasks.
	m_runInitCleanTasks = false;
	m_initCv.notify_all();
	m_cleanCv.notify_all();
	m_initTaskThread.join();
	m_cleanTaskThread.join();

	m_logger.Flush();
}


void GraphicsEngine::Update(float elapsed) {
	std::chrono::nanoseconds frameTime(long long(elapsed * 1e9));
	m_absoluteTime += frameTime;

	// Wait for previous frame on this BB to complete
	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	if (m_frameEndFenceValues[backBufferIndex].first != nullptr) {
		m_frameEndFenceValues[backBufferIndex].first->Wait(m_frameEndFenceValues[backBufferIndex].second);
	}

	// Set up context
	FrameContext context;
	context.frameTime = frameTime;
	context.absoluteTime = m_absoluteTime;
	context.log = &m_logStreamPipeline;

	context.gxApi = m_graphicsApi;
	context.commandAllocatorPool = &m_commandAllocatorPool;
	context.scratchSpacePool = &m_scratchSpacePool;

	context.commandQueue = &m_masterCommandQueue;
	context.backBuffer = &m_backBufferHeap->GetBackBuffer(backBufferIndex);
	context.scenes = &m_scenes;

	context.initMutex = &m_initMutex;
	context.cleanMutex = &m_cleanMutex;
	context.initQueue = &m_initTasks;
	context.cleanQueue = &m_cleanTasks;
	context.initCv = &m_initCv;
	context.cleanCv = &m_cleanCv;

	// Execute the pipeline
	m_scheduler.Execute(context);

	// Mark frame completion
	m_frameEndFenceValues[backBufferIndex] = m_masterCommandQueue.Signal();

	// Flush log
	m_logger.Flush();

	// Present frame
	m_swapChain->Present();
}




// Resources
TypedMesh* GraphicsEngine::CreateMesh() {
	return new TypedMesh();
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
	return new MeshEntity();
}



void GraphicsEngine::InitTaskThreadFunc() {
	bool notEmpty = false;
	while (m_runInitCleanTasks || notEmpty) {
		std::unique_lock<std::mutex> lkg(m_initMutex);

		m_initCv.wait(lkg, [this] {
			return !m_initTasks.empty() || !m_runInitCleanTasks;
		});

		if (!m_initTasks.empty()) {
			auto task = m_initTasks.front();
			m_initTasks.pop();
			notEmpty = !m_initTasks.empty();
			lkg.unlock();

			if (task.task) {
				task.task();
			}
			if (task.setThisFence != nullptr) {
				task.setThisFence->Signal(task.toThisValue);
			}
		}
	}
}


void GraphicsEngine::CleanTaskThreadFunc() {
	bool notEmpty = false;
	while (m_runInitCleanTasks || notEmpty) {
		std::unique_lock<std::mutex> lkg(m_cleanMutex);

		m_cleanCv.wait(lkg, [this] {
			return !m_cleanTasks.empty() || !m_runInitCleanTasks;
		});

		if (!m_cleanTasks.empty()) {
			auto task = m_cleanTasks.front();
			m_cleanTasks.pop();
			notEmpty = !m_cleanTasks.empty();
			lkg.unlock();

			if (task.waitThisFence != nullptr) {
				task.waitThisFence->Wait(task.toReachThisValue);
			}
			if (task.task) {
				task.task();
			}
		}
	}
}



void GraphicsEngine::CreatePipeline() {
	std::unique_ptr<nodes::FrameCounter> frameCounter(new nodes::FrameCounter());
	std::unique_ptr<nodes::FrameColor> frameColor(new nodes::FrameColor());
	std::unique_ptr<nodes::GetBackBuffer> getBackBuffer(new nodes::GetBackBuffer());
	std::unique_ptr<nodes::ClearRenderTarget> clearRtv(new nodes::ClearRenderTarget());
	std::unique_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::unique_ptr<nodes::TescoRender> renderWorld(new nodes::TescoRender());

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