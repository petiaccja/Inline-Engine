#include "PicoEngine.hpp"
#include "Geometry.hpp"
#include <BaseLibrary/Logging_All.hpp>

#include <GraphicsApi_LL/Exception.hpp>

#include <chrono>

#include <DirectXMath.h>

using namespace inl::gxapi;


PicoEngine::PicoEngine(inl::gxapi::NativeWindowHandle hWnd, int width, int height, exc::LogStream* logStream) :
	m_scissorRect{0, height, 0, width}
{
	// Set vars
	m_logStream = logStream;

	m_viewport.topLeftX = 0;
	m_viewport.topLeftY = 0;
	m_viewport.width = width;
	m_viewport.height = height;
	m_viewport.minDepth = 0;
	m_viewport.maxDepth = 1;

	m_width = width;
	m_height = height;


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
	m_currentBackBuffer = m_swapChain->GetCurrentBufferIndex();

	// Get swap chain buffers
	m_rtvs.reset(m_graphicsApi->CreateDescriptorHeap(DescriptorHeapDesc{ eDesriptorHeapType::RTV, 2, false }));
	RenderTargetViewDesc rtvDesc;
	rtvDesc.format = eFormat::R8G8B8A8_UNORM;
	rtvDesc.dimension = eRtvDimension::TEXTURE2D;
	rtvDesc.tex2D.firstMipLevel = 0;
	rtvDesc.tex2D.planeIndex = 0;
	IResource* rt1 = m_swapChain->GetBuffer(0);
	IResource* rt2 = m_swapChain->GetBuffer(1);
	m_graphicsApi->CreateRenderTargetView(rt1, rtvDesc, m_rtvs->At(0));
	m_graphicsApi->CreateRenderTargetView(rt2, rtvDesc, m_rtvs->At(1));

	m_dsv.reset(m_graphicsApi->CreateDescriptorHeap(DescriptorHeapDesc{ eDesriptorHeapType::DSV, 2, false }));
	{
		ResourceDesc depthBufferDesc;
		depthBufferDesc.type = eResourceType::TEXTURE;
		depthBufferDesc.textureDesc.alignment = 0;
		depthBufferDesc.textureDesc.depthOrArraySize = 1;
		depthBufferDesc.textureDesc.dimension = eTextueDimension::TWO;
		depthBufferDesc.textureDesc.flags = eResourceFlags::ALLOW_DEPTH_STENCIL;
		depthBufferDesc.textureDesc.format = eFormat::D32_FLOAT;
		depthBufferDesc.textureDesc.height = m_viewport.height;
		depthBufferDesc.textureDesc.width = m_viewport.width;
		depthBufferDesc.textureDesc.layout = eTextureLayout::UNKNOWN;
		depthBufferDesc.textureDesc.mipLevels = 1;
		depthBufferDesc.textureDesc.multisampleCount = 1;
		depthBufferDesc.textureDesc.multisampleQuality = 0;
		ClearValue depthClearValue{ eFormat::D32_FLOAT, 1.0, 0 };
		m_depthBuffers.push_back(std::move(std::unique_ptr<IResource>(m_graphicsApi->CreateCommittedResource(HeapProperties{}, {}, depthBufferDesc, eResourceState::GENERIC_READ, &depthClearValue))));
		m_depthBuffers.push_back(std::move(std::unique_ptr<IResource>(m_graphicsApi->CreateCommittedResource(HeapProperties{}, {}, depthBufferDesc, eResourceState::GENERIC_READ, &depthClearValue))));

		m_graphicsApi->CreateDepthStencilView(m_depthBuffers[0].get(), m_dsv->At(0));
		m_graphicsApi->CreateDepthStencilView(m_depthBuffers[1].get(), m_dsv->At(1));
	}


	// Create root signature
	DescriptorRange descriptorRanges[1] = {
		DescriptorRange{DescriptorRange::SRV, 1, 0, 0},
	};
	RootParameterDesc rootParameters[3] = {
		RootParameterDesc::Constant(32, 0),
		RootParameterDesc::Cbv(8),
		RootParameterDesc::DescriptorTable(1, descriptorRanges),
	};
	m_defaultRootSignature.reset(m_graphicsApi->CreateRootSignature(RootSignatureDesc{3, rootParameters}));

	//Compile shaders
	ShaderProgramBinary vertexBinary;
	ShaderProgramBinary pixelBinary;

	std::string errorMsg;
	Log("Loading vertex shader.");
	vertexBinary = m_gxapiManager->CompileShaderFromFile("shaders.hlsl", "VSmain", inl::gxapi::eShaderType::VERTEX, eShaderCompileFlags{}, {});
	
	Log("Loading fragment shader.");
	pixelBinary = m_gxapiManager->CompileShaderFromFile("shaders.hlsl", "PSmain", inl::gxapi::eShaderType::PIXEL, eShaderCompileFlags{}, {});

	std::stringstream ss_;
	ss_ << "sizeof vertex = " << sizeof(Vertex);
	Log(ss_.str());

	// Create default PSO
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

		InputElementDesc ilNormalDesc;
		ilNormalDesc.semanticName = "NORMAL";
		ilNormalDesc.semanticIndex = 0;
		ilNormalDesc.format = eFormat::R32G32B32_FLOAT;
		ilNormalDesc.inputSlot = 0;
		ilNormalDesc.offset = 3*sizeof(float);
		ilNormalDesc.classifiacation = eInputClassification::VERTEX_DATA;
		ilNormalDesc.instanceDataStepRate = 0;
		ilElements.push_back(ilNormalDesc);

		InputElementDesc ilTexDesc;
		ilTexDesc.semanticName = "TEX";
		ilTexDesc.semanticIndex = 0;
		ilTexDesc.format = eFormat::R32G32_FLOAT;
		ilTexDesc.inputSlot = 0;
		ilTexDesc.offset = 6 * sizeof(float);
		ilTexDesc.classifiacation = eInputClassification::VERTEX_DATA;
		ilTexDesc.instanceDataStepRate = 0;
		ilElements.push_back(ilTexDesc);
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

	GraphicsPipelineStateDesc psoDesc{};
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = eFormat::R8G8B8A8_UNORM;
	psoDesc.rootSignature = m_defaultRootSignature.get();
	psoDesc.vs = ToShaderByteCodeDesc(vertexBinary);
	psoDesc.ps = ToShaderByteCodeDesc(pixelBinary);
	psoDesc.inputLayout = inputLayout;
	psoDesc.rasterization = rasterizationState;
	psoDesc.blending = blendState;
	psoDesc.depthStencilState.enableDepthTest = true;
	psoDesc.depthStencilState.enableStencilTest = false;
	psoDesc.depthStencilState.enableDepthStencilWrite = true;
	psoDesc.depthStencilFormat = eFormat::D32_FLOAT;
	psoDesc.primitiveTopologyType = ePrimitiveTopologyType::TRIANGLE;
	m_defaultPso.reset(m_graphicsApi->CreateGraphicsPipelineState(psoDesc));

	// Create command allocator and command list
	Log("Creating command allocator.");
	m_commandAllocator.reset(m_graphicsApi->CreateCommandAllocator(eCommandListType::GRAPHICS));
	Log("Creating graphics command list.");
	m_commandList.reset(m_graphicsApi->CreateGraphicsCommandList(CommandListDesc{ m_commandAllocator.get(), m_defaultPso.get() }));
	// Command Lists are in recording state when created. In order
	// to use CommandAllocator::Reset() later in the update
	// function, the command list must first be closed.
	m_commandList->Close();

	// Create a fence
	m_fence.reset(m_graphicsApi->CreateFence(0));

	
	// Create geometry buffers
	Geometry cube = Geometry::CreatCube();

	const MemoryRange noReadRange{0, 0};
	{
		ResourceDesc vertexBufferDesc;
		vertexBufferDesc.type = eResourceType::BUFFER;
		vertexBufferDesc.bufferDesc.sizeInBytes = sizeof(Vertex) * cube.GetNumVertices();
		m_vertexBuffer.reset(m_graphicsApi->CreateCommittedResource(HeapProperties{eHeapType::UPLOAD}, eHeapFlags::NONE, vertexBufferDesc, eResourceState::GENERIC_READ));

		void* mappedVertexData = m_vertexBuffer->Map(0, &noReadRange);
		memcpy(mappedVertexData, cube.GetVertices(), vertexBufferDesc.bufferDesc.sizeInBytes);
		m_vertexBuffer->Unmap(0, nullptr);

		vbv.gpuAddress = m_vertexBuffer->GetGPUAddress();
		vbv.size = vertexBufferDesc.bufferDesc.sizeInBytes;
		vbv.stride = sizeof(Vertex);
	}
	{
		ResourceDesc indexBufferDesc;
		indexBufferDesc.type = eResourceType::BUFFER;
		indexBufferDesc.bufferDesc.sizeInBytes = sizeof(uint16_t) * cube.GetNumIndices();
		m_indexBuffer.reset(m_graphicsApi->CreateCommittedResource(HeapProperties{eHeapType::UPLOAD}, eHeapFlags::NONE, indexBufferDesc, eResourceState::GENERIC_READ));

		void* mappedIndexData = m_indexBuffer->Map(0, &noReadRange);
		memcpy(mappedIndexData, cube.GetIndices(), indexBufferDesc.bufferDesc.sizeInBytes);
		m_indexBuffer->Unmap(0, nullptr);

		ibv.gpuAddress = m_indexBuffer->GetGPUAddress();
		ibv.format = eFormat::R16_UINT;
		ibv.size = indexBufferDesc.bufferDesc.sizeInBytes;
	}

	//wait for command queue to finish
	uint64_t prevValue = m_fence->Fetch();
	m_commandQueue->Signal(m_fence.get(), prevValue + 1);
	m_fence->Wait(prevValue + 1);
}

PicoEngine::~PicoEngine() {
	Log("PicoEngine going down...");
}


void PicoEngine::Update() {
	static std::chrono::high_resolution_clock::time_point startTime;
	double elapsedTotal = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;;


	// query current back buffer id
	m_currentBackBuffer = m_swapChain->GetCurrentBufferIndex();

	// reset command allocator and list
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.get(), m_defaultPso.get());

	m_commandList->SetViewports(1, &m_viewport);
	m_commandList->SetScissorRects(1, &m_scissorRect);

	DescriptorHandle currRenderTarget = m_rtvs->At(m_currentBackBuffer % 2);
	DescriptorHandle currDepthStencil = m_dsv->At(m_currentBackBuffer % 2);
	m_commandList->SetRenderTargets(1, &currRenderTarget, &currDepthStencil);

	// init drawing
	m_commandList->ClearRenderTarget(currRenderTarget, ColorRGBA(0.2, 0.2, 0.7));
	m_commandList->ClearDepthStencil(currDepthStencil, 1, 0);

	m_commandList->SetPrimitiveTopology(ePrimitiveTopology::TRIANGLELIST);
	m_commandList->SetGraphicsRootSignature(m_defaultRootSignature.get());
	// draw
	{
		double angle;
		double revs;
		angle = std::modf(elapsedTotal*0.3, &revs);

		DirectX::XMMATRIX world, view, proj;
		world = DirectX::XMMatrixRotationZ(angle*3.1415926*2);
		view = DirectX::XMMatrixLookAtRH({ 4,2,2}, { 0,0,0 }, { 0,0,1 });
		proj = DirectX::XMMatrixPerspectiveFovRH((50.f / 180) * 3.1415926, (float)m_width/m_height, 0.1, 100);
		DirectX::XMMATRIX viewProj = view * proj;
		m_commandList->SetGraphicsRootConstants(0, 0, 16, (uint32_t*)&world);
		m_commandList->SetGraphicsRootConstants(0, 16, 16, (uint32_t*)&viewProj);

		m_commandList->SetVertexBuffers(0, 1, &vbv.gpuAddress, &vbv.size, &vbv.stride);
		m_commandList->SetIndexBuffer(ibv.gpuAddress, ibv.size, ibv.format);
		m_commandList->DrawIndexedInstanced(ibv.size / sizeof(std::uint16_t));
	}
	//m_commandList->DrawInstanced(3);

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

