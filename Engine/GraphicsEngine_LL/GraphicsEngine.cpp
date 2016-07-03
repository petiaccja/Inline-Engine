#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging


namespace inl {
namespace gxeng {

using namespace gxapi;



GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc)
	: m_commandAllocatorPool(desc.graphicsApi),
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

	// Do more stuff...
	CreatePipeline();
	m_scheduler.SetPipeline(std::move(m_pipeline));

	// Launch init and clean tasks
	m_runInitCleanTasks = true;
	m_initTaskThread = std::thread(std::bind(&GraphicsEngine::InitTaskThreadFunc, this));
	m_cleanTaskThread = std::thread(std::bind(&GraphicsEngine::CleanTaskThreadFunc, this));
}


GraphicsEngine::~GraphicsEngine() {
	m_runInitCleanTasks = false;
	m_initCv.notify_all();
	m_cleanCv.notify_all();
	m_cleanTaskThread.join();
	m_initTaskThread.join();
}


void GraphicsEngine::Update(float elapsed) {
	FrameContext context;
	context.commandAllocatorPool = &m_commandAllocatorPool;
	context.commandQueue = &m_masterCommandQueue;
	context.frameTime = elapsed;
	context.initMutex = &m_initMutex;
	context.cleanMutex = &m_cleanMutex;
	context.initQueue = &m_initTasks;
	context.cleanQueue = &m_cleanTasks;
	context.initCv = &m_initCv;
	context.cleanCv = &m_cleanCv;
	m_scheduler.Execute(context);
}



void GraphicsEngine::InitTaskThreadFunc() {
	while (m_runInitCleanTasks) {
		std::unique_lock<std::mutex> lkg(m_initMutex);

		m_initCv.wait(lkg, [this] {
			return !m_initTasks.empty() || !m_runInitCleanTasks;
		});

		if (!m_initTasks.empty()) {
			auto task = m_initTasks.front();
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
	while (m_runInitCleanTasks) {
		std::unique_lock<std::mutex> lkg(m_cleanMutex);

		m_cleanCv.wait(lkg, [this] {
			return !m_cleanTasks.empty() || !m_runInitCleanTasks;
		});

		if (!m_cleanTasks.empty()) {
			auto task = m_cleanTasks.front();
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
	// Test node
	class Node : public GraphicsNode,
		public exc::InputPortConfig<int, int, int>,
		public exc::OutputPortConfig<int, int, int>
	{
	public:
		Node() {
			GetInput<0>().Set(0);
			GetInput<1>().Set(0);
			GetInput<2>().Set(0);
		}
		virtual void Update() override {
			std::cout << this << " updated." << std::endl;
		}
		virtual void Notify(exc::InputPortBase* sender) override {
			std::cout << this << " notified." << std::endl;
		}
		virtual Task GetTask() override {
			Task t;
			t.InitSequential({
				[this](ExecutionContext context)
			{
				if (this->GetInput<0>().Get() == 0) {
					++frameCount;
					this->GetOutput<0>().Set(frameCount);
				}
				else {
					std::cout << "Frame " << this->GetInput<0>().Get() << std::endl;
				}
				return ExecutionResult();
			}
			});
			return t;
		}
	private:
		unsigned long long frameCount = 0;
	};

	Node *n1 = new Node(), *n2 = new Node();

	n1->GetOutput(0)->Link(n2->GetInput(0));
	m_pipeline.CreateFromNodesList({ n1, n2 }, std::default_delete<exc::NodeBase>());
}


} // namespace gxeng
} // namespace inl