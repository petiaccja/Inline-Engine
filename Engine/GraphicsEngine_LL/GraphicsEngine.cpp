#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging


namespace inl {
namespace gxeng {

using namespace gxapi;



GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc) 
	: m_commandAllocatorPool(desc.graphicsApi.get()),
	m_gxapiManager(std::move(desc.gxapiManager)),
	m_graphicsApi(std::move(desc.graphicsApi))
{
	// Create default graphics command queue
	m_mainCommandQueue.reset(m_graphicsApi->CreateCommandQueue(CommandQueueDesc{ eCommandListType::GRAPHICS }));

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
	m_swapChain.reset(m_gxapiManager->CreateSwapChain(swapChainDesc, m_mainCommandQueue.get()));

	// Do more stuff...
	CreatePipeline();
	m_scheduler.SetPipeline(std::move(m_pipeline));
}



void GraphicsEngine::CreatePipeline() {
	// Test node
	class Node : public GraphicsNode,
		public exc::InputPortConfig<int, int, int>,
		public exc::OutputPortConfig<int, int, int>
	{
	public:
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
					std::cout << this << " task running." << std::endl;
					return ExecutionResult();
				}
			});
			return t;
		}
	};

	Node *n1 = new Node(), *n2 = new Node();
	n1->GetOutput(0)->Link(n2->GetInput(0));
	m_pipeline.CreateFromNodesList({ n1, n2 }, std::default_delete<exc::NodeBase>());
}



void GraphicsEngine::Update(float elapsed) {
	FrameContext context;
	context.commandAllocatorPool = &m_commandAllocatorPool;
	context.commandQueue = m_mainCommandQueue.get();
	context.frameTime = elapsed;
	m_scheduler.Execute(context);
}



} // namespace gxeng
} // namespace inl