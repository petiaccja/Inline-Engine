#include "PicoEngine.hpp"
#include <BaseLibrary/Logging>

#include <GraphicsApi_LL/Exception.hpp>

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

	//Compile shaders
	ShaderProgramBinary vertexBinary;
	ShaderProgramBinary pixelBinary;

	std::string errorMsg;
	Log("Loading vertex shader.");
	if (!m_gxapiManager->CompileShaderFromFile("shaders.hlsl", "VSmain", inl::gxapi::eShaderType::VERTEX, eShaderCompileFlags{}, {}, vertexBinary, errorMsg)) {
		throw Exception("Could not compile shader. Message: " + errorMsg);
	}
	Log("Loading fragment shader.");
	if (!m_gxapiManager->CompileShaderFromFile("shaders.hlsl", "PSmain", inl::gxapi::eShaderType::PIXEL, eShaderCompileFlags{}, {}, pixelBinary, errorMsg)) {
		throw Exception("Could not compile shader. Message: " + errorMsg);
	}

	InputLayout inputLayout;
	std::vector<InputElementDesc> ilElements;
	{
		InputElementDesc ilPosDesc;
		ilPosDesc.semanticName = "POSITION";
		ilPosDesc.semanticIndex = 0;
		ilPosDesc.format = eFormat::R32G32B32_FLOAT;
		ilPosDesc.inputSlot = 0;
		ilPosDesc.offset = 0;
		ilPosDesc.classifiacation = eInputClassification::VERTEX_DATA;
		ilPosDesc.instanceDataStepRate = 0;
		ilElements.push_back(ilPosDesc);

		InputElementDesc ilColDesc;
		ilColDesc.semanticName = "COLOR";
		ilColDesc.semanticIndex = 0;
		ilColDesc.format = eFormat::R32G32B32_FLOAT;
		ilColDesc.inputSlot = 0;
		ilColDesc.offset = 3*sizeof(float);
		ilColDesc.classifiacation = eInputClassification::VERTEX_DATA;
		ilColDesc.instanceDataStepRate = 0;
		ilElements.push_back(ilColDesc);
	}
	inputLayout.elements = ilElements.data();
	inputLayout.numElements = ilElements.size();

	RasterizerState rasterizationState;
	rasterizationState.fillMode = eFillMode::SOLID;
	rasterizationState.cullMode = eCullMode::DRAW_ALL;
	rasterizationState.depthBias = 0;
	rasterizationState.depthBiasClamp = 0;
	rasterizationState.slopeScaledDepthBias = 0;
	rasterizationState.depthClipEnabled = true;
	rasterizationState.multisampleEnabled = false;
	rasterizationState.lineAntialiasingEnabled = false;
	rasterizationState.forcedSampleCount = 0;
	rasterizationState.conservativeRasterization = eConservativeRasterizationMode::OFF;

	BlendState blendState;
	blendState.alphaToCoverage = false;
	blendState.independentBlending = false;
	size_t targetCount = sizeof(blendState.multiTarget) / sizeof(blendState.multiTarget[0]);
	for (UINT i = 0; i < targetCount; ++i) {
		blendState.multiTarget[i].enableBlending = false;
		blendState.multiTarget[i].enableLogicOp = false;
		blendState.multiTarget[i].colorOperand1 = eBlendOperand::ONE;
		blendState.multiTarget[i].colorOperand2 = eBlendOperand::ZERO;
		blendState.multiTarget[i].colorOperation = eBlendOperation::ADD;
		blendState.multiTarget[i].alphaOperand1 = eBlendOperand::ONE;
		blendState.multiTarget[i].alphaOperand2 = eBlendOperand::ZERO;
		blendState.multiTarget[i].alphaOperation = eBlendOperation::ADD;
		blendState.multiTarget[i].logicOperation = eBlendLogicOperation::NOOP;
		blendState.multiTarget[i].mask = eColorMask::ALL;
	}

	// Create default PSO
	GraphicsPipelineStateDesc psoDesc{};
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = eFormat::R8G8B8A8_UNORM;
	psoDesc.rootSignature = m_defaultRootSignature.get();
	psoDesc.vs = ToShaderByteCodeDesc(vertexBinary);
	psoDesc.ps = ToShaderByteCodeDesc(pixelBinary);
	psoDesc.inputLayout = inputLayout;
	psoDesc.rasterization = rasterizationState;
	psoDesc.blending = blendState;
	psoDesc.depthStencilState.enableDepthTest = false;
	psoDesc.depthStencilState.enableStencilTest = false;
	psoDesc.blendSampleMask = UINT_MAX;
	psoDesc.primitiveTopologyType = ePrimitiveTopologyType::TRIANGLE;
	m_defaultPso.reset(m_graphicsApi->CreateGraphicsPipelineState(psoDesc));

	// Create command allocator and command list
	m_commandAllocator.reset(m_graphicsApi->CreateCommandAllocator(eCommandListType::GRAPHICS));
	m_commandList.reset(m_graphicsApi->CreateGraphicsCommandList(CommandListDesc{ m_commandAllocator.get(), m_defaultPso.get() }));

	// Create a fence
	m_fence.reset(m_graphicsApi->CreateFence(0));

	
	//Create an object to be drawn
	/*
	//////////////////////
	// CUBE
	static float vertices[] = {
		// top
		0.5, 0.5, 0.5,		0,0,1,	1,0,
		-0.5, 0.5, 0.5,		0,0,1,	1,1,
		-0.5, -0.5, 0.5,	0,0,1,	0,1,
		0.5, -0.5, 0.5,		0,0,1,	0,0,
		// bottom
		0.5, 0.5, -0.5,		0,0,-1,	1,0,
		-0.5, 0.5, -0.5,	0,0,-1,	1,1,
		-0.5, -0.5, -0.5,	0,0,-1,	0,1,
		0.5, -0.5, -0.5,	0,0,-1,	0,0,
		// right
		0.5, 0.5, 0.5,		1,0,0,	1,0,
		0.5, -0.5, 0.5,		1,0,0,	1,1,
		0.5, -0.5, -0.5,	1,0,0,	0,1,
		0.5, 0.5, -0.5,		1,0,0,	0,0,
		// left
		-0.5, 0.5, 0.5,		-1,0,0,	1,0,
		-0.5, -0.5, 0.5,	-1,0,0,	1,1,
		-0.5, -0.5, -0.5,	-1,0,0,	0,1,
		-0.5, 0.5, -0.5,	-1,0,0,	0,0,
		// front
		0.5, 0.5, 0.5,		0,1,0,	1,0,
		0.5, 0.5, -0.5,		0,1,0,	1,1,
		-0.5, 0.5, -0.5,	0,1,0,	0,1,
		-0.5, 0.5, 0.5,		0,1,0,	0,0,
		// back
		0.5, -0.5, 0.5,		0,-1,0,	1,0,
		0.5, -0.5, -0.5,	0,-1,0,	1,1,
		-0.5, -0.5, -0.5,	0,-1,0,	0,1,
		-0.5, -0.5, 0.5,	0,-1,0,	0,0,
	};

	static int indices[] = {
		// top
		0,1,3,
		1,2,3,
		// bottom
		5,4,7,
		6,5,7,
		// right
		8,9,11,
		9,10,11,
		// left
		13,12,15,
		14,13,15,
		// front
		16,17,19,
		17,18,19,
		// back
		21,20,23,
		22,21,23,
	};*/

	//////////
	// TRIANGLE
	static float vertices[] = {
		//pos              color
		-0.5, -0.5, 0,     1, 0, 0,
		 0.5, -0.5, 0,     0, 1, 0,
		 0.0,  0.5, 0,     0, 0, 1
	};

	static std::uint32_t indices[] = {
		0, 1, 2
	};

	const MemoryRange noReadRange{0, 0};
	{
		ResourceDesc vertexBufferDesc;
		vertexBufferDesc.type = eResourceType::BUFFER;
		vertexBufferDesc.bufferDesc.sizeInBytes = sizeof(vertices);
		m_vertexBuffer.reset(m_graphicsApi->CreateCommittedResource(HeapProperties{}, eHeapFlags::NONE, vertexBufferDesc, eResourceState::GENERIC_READ));

		void* mappedVertexData = m_vertexBuffer->Map(0, &noReadRange);
		memcpy(mappedVertexData, vertices, sizeof(vertices));
		m_vertexBuffer->Unmap(0, nullptr);

		vbv.gpuAddress = m_vertexBuffer->GetGPUAddress();
		vbv.size = sizeof(vertices);
		vbv.stride = sizeof(float) * (3+3);
	}
	{
		ResourceDesc indexBufferDesc;
		indexBufferDesc.type = eResourceType::BUFFER;
		indexBufferDesc.bufferDesc.sizeInBytes = sizeof(indices);
		m_indexBuffer.reset(m_graphicsApi->CreateCommittedResource(HeapProperties{}, eHeapFlags::NONE, indexBufferDesc, eResourceState::GENERIC_READ));

		void* mappedIndexData = m_indexBuffer->Map(0, &noReadRange);
		memcpy(mappedIndexData, indices, sizeof(indices));
		m_indexBuffer->Unmap(0, nullptr);

		ibv.gpuAddress = m_indexBuffer->GetGPUAddress();
		ibv.format = eFormat::R32_UINT;
		ibv.size = sizeof(indices);
	}
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
	m_commandList->SetPrimitiveTopology(ePrimitiveTopology::TRIANGLELIST);
	m_commandList->SetVertexBuffers(0, 1, &vbv.gpuAddress, &vbv.size, &vbv.stride);
	m_commandList->SetIndexBuffer(ibv.gpuAddress, ibv.size, ibv.format);
	m_commandList->DrawIndexedInstanced(ibv.size / sizeof(std::uint32_t));

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



ShaderByteCodeDesc PicoEngine::ToShaderByteCodeDesc(const ShaderProgramBinary& binary) {
	ShaderByteCodeDesc result;

	result.shaderByteCode = binary.data.data();
	result.sizeOfByteCode = binary.data.size();

	return result;
}


void PicoEngine::Log(std::string s) {
	if (m_logStream) m_logStream->Event(s);
}

