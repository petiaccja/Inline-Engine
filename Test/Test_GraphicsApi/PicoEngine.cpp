#include "PicoEngine.hpp"
#include <BaseLibrary/Logging>


using namespace inl::gxapi;


PicoEngine::PicoEngine(inl::gxapi::NativeWindowHandle hWnd, int width, int height, exc::LogStream* logStream) {
	// Set vars
	m_logStream = logStream;


	// Check adapter list
	m_gxapiManager.reset(new inl::gxapi_dx12::GxapiManager());
	Log("GxApi Manager created.");


	auto adapters = m_gxapiManager->EnumerateAdapters();
	std::stringstream ss;
	ss << "Enumerating adapters...";
	for (auto& adapter : adapters) {
		ss << "\n" << adapter.name << std::endl
			<< "   " << "Memory     = " << adapter.dedicatedVideoMemory / 1048576 << " MiB" << std::endl
			<< "   " << "Sys. mem.  = " << adapter.dedicatedSystemMemory / 1048576 << " MiB" << std::endl
			<< "   " << "Sh. mem.   = " << adapter.sharedSystemMemory / 1048576 << " MiB" << std::endl
			<< "   " << "Type       = " << (adapter.isSoftwareAdapter ? "software" : "hardware");
	}
	Log(ss.str());

	// Create device
	if (adapters.size() == 0) {
		throw std::runtime_error("No available devices.");
	}
	m_graphicsApi.reset(m_gxapiManager->CreateGraphicsApi(adapters[0].adapterId));
	Log(exc::Event{
			"Device created for first adapter.",
			exc::eEventType::INFO,
			exc::EventParameterInt("adapter_id", adapters[0].adapterId),
			exc::EventParameterString("adapter_name", adapters[0].name)
	});

	// Create command queue
	CommandQueueDesc commandQueueDesc{ eCommandListType::GRAPHICS };
	m_commandQueue.reset(m_graphicsApi->CreateCommandQueue(commandQueueDesc));

	// Create swap chain
	SwapChainDesc swapChainDesc;
	swapChainDesc.isFullScreen = false;
	swapChainDesc.format = eFormat::R8G8B8A8_UNORM;
	swapChainDesc.width = width;
	swapChainDesc.height = height;
	swapChainDesc.numBuffers = 2;
	swapChainDesc.targetWindow = hWnd;
	swapChainDesc.multisampleCount = 1;
	swapChainDesc.multiSampleQuality = 0;
	m_swapChain.reset(m_gxapiManager->CreateSwapChain(swapChainDesc, m_commandQueue.get()));
	currentBackBuffer = m_swapChain->GetCurrentBufferIndex();

	// Get swap chain buffers
	m_rtvs.reset(m_graphicsApi->CreateDescriptorHeap(DescriptorHeapDesc{ eDesriptorHeapType::RTV, 2, false }));
	RenderTargetViewDesc rtvDesc;
	rtvDesc.format = eFormat::R8G8B8A8_UNORM;
	rtvDesc.dimension = eRtvDimension::TEXTURE2D;
	rtvDesc.tex2D.firstMipLevel = 0;
	rtvDesc.tex2D.planeIndex = 0;
	m_graphicsApi->CreateRenderTargetView(rtvDesc, m_rtvs->At(0));
	m_graphicsApi->CreateRenderTargetView(rtvDesc, m_rtvs->At(1));

	// Create root signature
	m_defaultRootSignature.reset(m_graphicsApi->CreateRootSignature(RootSignatureDesc{}));

	// Create default PSO
	GraphicsPipelineStateDesc psoDesc{};
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = eFormat::R8G8B8A8_UNORM;
	psoDesc.rootSignature = m_defaultRootSignature.get();
	m_defaultPso.reset(m_graphicsApi->CreateGraphicsPipelineState(psoDesc));

	// Create command allocator and command list
	m_commandAllocator.reset(m_graphicsApi->CreateCommandAllocator(eCommandListType::GRAPHICS));
	m_commandList.reset(m_graphicsApi->CreateGraphicsCommandList(CommandListDesc{ m_commandAllocator.get(), m_defaultPso.get() }));

	// Create a fence
	m_fence.reset(m_graphicsApi->CreateFence(0));
}

PicoEngine::~PicoEngine() {
	Log("PicoEngine going down...");
}


void PicoEngine::Update() {
	// reset command list
	m_commandList->Reset(m_commandAllocator.get(), m_defaultPso.get());

	// init drawing
	m_commandList->ClearRenderTarget(m_rtvs->At(currentBackBuffer), ColorRGBA(0.2, 0.2, 0.7));
	
	// draw
	// ...

	// close command list
	m_commandList->Close();


	// commit to queue
	ICommandList* lists[] = { m_commandList.get() };
	m_commandQueue->ExecuteCommandLists(1, lists);

	// wait for frame to finish
	uint64_t prevValue = m_fence->Fetch();
	m_commandQueue->Signal(m_fence.get(), prevValue + 1);
	m_fence->Wait(prevValue + 1);

	// present frame
	m_swapChain->Present();
}




void PicoEngine::Log(std::string s) {
	if (m_logStream) m_logStream->Event(s);
}