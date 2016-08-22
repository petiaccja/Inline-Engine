#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging

#include "Nodes/ClearScreen.h"
#include "Nodes/FrameCounter.hpp"
#include "Nodes/FrameColor.hpp"


namespace inl {
namespace gxeng {

using namespace gxapi;



GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc)
	: m_commandAllocatorPool(desc.graphicsApi),
	m_scratchSpacePool(desc.graphicsApi),
	m_gxapiManager(desc.gxapiManager),
	m_graphicsApi(desc.graphicsApi),
	m_masterCommandQueue(desc.graphicsApi->CreateCommandQueue(CommandQueueDesc{ eCommandListType::GRAPHICS }), desc.graphicsApi->CreateFence(0))
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

	m_frameEndFenceValues.resize(m_swapChain->GetDesc().numBuffers, 0);

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
}


GraphicsEngine::~GraphicsEngine() {
	// Flush all the things.
	m_masterCommandQueue.GetFence()->Signal(std::numeric_limits<uint64_t>::max());

	// Stop tasks.
	m_runInitCleanTasks = false;
	m_initCv.notify_all();
	m_cleanCv.notify_all();
	m_initTaskThread.join();
	m_cleanTaskThread.join();

	m_logger.Flush();
}


void GraphicsEngine::Update(float elapsed) {
	// Wait for previous frame on this BB to complete
	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	m_masterCommandQueue.GetFence()->Wait(m_frameEndFenceValues[backBufferIndex]);

	// Set up context
	FrameContext context;
	context.frameTime = elapsed;
	context.log = &m_logStreamPipeline;

	context.gxApi = m_graphicsApi;
	context.commandAllocatorPool = &m_commandAllocatorPool;
	context.scratchSpacePool = &m_scratchSpacePool;

	context.commandQueue = &m_masterCommandQueue;
	context.initMutex = &m_initMutex;
	context.cleanMutex = &m_cleanMutex;
	context.initQueue = &m_initTasks;
	context.cleanQueue = &m_cleanTasks;
	context.initCv = &m_initCv;
	context.cleanCv = &m_cleanCv;

	// TEMPORARY: set screen to clear
	Texture2D* backBuffer = &m_backBufferHeap->GetBackBuffer(backBufferIndex);
	m_clearScreen->SetTarget(backBuffer);

	// Execute the pipeline
	m_scheduler.Execute(context);

	// Mark frame completion
	m_frameEndFenceValues[backBufferIndex] = m_masterCommandQueue.IncrementFenceValue();
	m_masterCommandQueue.Signal(m_masterCommandQueue.GetFence(), m_frameEndFenceValues[backBufferIndex]);

	// Flush log
	m_logger.Flush();

	// Present frame
	m_swapChain->Present();
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
	auto* n1 = new FrameCounter();
	auto* n2 = new FrameColor();
	auto* n3 = new ClearScreen();
	m_clearScreen = n3;
	n1->SetLog(&m_logStreamGeneral);

	n1->GetOutput(0)->Link(n2->GetInput(0));
	n2->GetOutput(0)->Link(n3->GetInput(0));

	m_pipeline.CreateFromNodesList({ n1, n2, n3 }, std::default_delete<exc::NodeBase>());
}


} // namespace gxeng
} // namespace inl