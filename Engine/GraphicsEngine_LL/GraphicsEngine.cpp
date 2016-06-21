#include "GraphicsEngine.hpp"

#include "../GraphicsApi_D3D12/GxapiManager.hpp"


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
}



} // namespace gxeng
} // namespace inl