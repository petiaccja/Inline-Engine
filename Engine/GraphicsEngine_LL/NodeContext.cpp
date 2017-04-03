#include "NodeContext.hpp"

#include "MemoryManager.hpp" 
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"
#include "GraphicsCommandList.hpp"


namespace inl::gxeng {



//------------------------------------------------------------------------------
// Engine Context
//------------------------------------------------------------------------------

EngineContext::EngineContext(int cpuCount = 1, int gpuCount = 1) {
	m_cpuCount = cpuCount;
	m_gpuCount = gpuCount;
}


int EngineContext::GetProcessorCoreCount() const {
	return m_cpuCount;
}

int EngineContext::GetGraphicsDeviceCount() const {
	return m_gpuCount;
}



//------------------------------------------------------------------------------
// Setup Context
//------------------------------------------------------------------------------

SetupContext::SetupContext(MemoryManager* memoryManager = nullptr,
						   CbvSrvUavHeap* srvHeap = nullptr,
						   RTVHeap* rtvHeap = nullptr,
						   DSVHeap* dsvHeap = nullptr,
						   ShaderManager* shaderManager = nullptr,
						   gxapi::IGraphicsApi* graphicsApi = nullptr)
	: m_memoryManager(memoryManager),
	m_srvHeap(srvHeap),
	m_rtvHeap(rtvHeap),
	m_dsvHeap(dsvHeap),
	m_shaderManager(shaderManager),
	m_graphicsApi(graphicsApi)
{}


Texture2D SetupContext::CreateTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, gxapi::eResourceFlags::NONE, arraySize);
	return texture;
}


Texture2D SetupContext::CreateRenderTarget2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_RENDER_TARGET;

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, flags, arraySize);
	return texture;
}


Texture2D SetupContext::CreateDepthStencil2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool shaderResource, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL;
	if (!shaderResource) {
		flags += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
	}
	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, flags, arraySize);
	return texture;
}


Texture2D SetupContext::CreateRWTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool renderTarget, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS;
	if (renderTarget) { flags += gxapi::eResourceFlags::ALLOW_RENDER_TARGET; }

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, flags, arraySize);
	return texture;
}


TextureView2D SetupContext::CreateSrv(Texture2D& texture, gxapi::eFormat format, gxapi::SrvTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create srv without srv/cbv/uav heap.");

	return TextureView2D{ texture, *m_srvHeap, format, desc };
}


RenderTargetView2D SetupContext::CreateRtv(Texture2D& texture, gxapi::eFormat format, gxapi::RtvTexture2DArray desc) const {
	if (m_rtvHeap == nullptr) throw std::logic_error("Cannot create rtv without rtv heap.");

	return RenderTargetView2D{ texture, *m_rtvHeap, format, desc };
}


DepthStencilView2D SetupContext::CreateDsv(Texture2D& texture, gxapi::eFormat format, gxapi::DsvTexture2DArray desc) const {
	if (m_dsvHeap == nullptr) throw std::logic_error("Cannot create dsv without dsv heap.");

	return DepthStencilView2D{ texture, *m_dsvHeap, format, desc };
}


RWTextureView2D SetupContext::CreateUav(Texture2D& rwTexture, gxapi::eFormat format, gxapi::UavTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create uav wihtout srv/cbv/uav heap.");

	return RWTextureView2D{ rwTexture, *m_srvHeap, format, desc };
}


VertexBuffer SetupContext::CreateVertexBuffer(const void* data, size_t size) const {
	VertexBuffer result = m_memoryManager->CreateVertexBuffer(eResourceHeapType::CRITICAL, size);
	m_memoryManager->GetUploadManager().Upload(result, 0, data, size);
	return result;
}


IndexBuffer SetupContext::CreateIndexBuffer(const void* data, size_t size, size_t indexCount) const {
	IndexBuffer result = m_memoryManager->CreateIndexBuffer(eResourceHeapType::CRITICAL, size, indexCount);
	m_memoryManager->GetUploadManager().Upload(result, 0, data, size);
	return result;
}

ConstBufferView SetupContext::CreateCbv(VolatileConstBuffer& buffer, size_t offset, size_t size, VolatileViewHeap& viewHeap) const {
	return ConstBufferView(
		buffer,
		viewHeap.Allocate(),
		m_graphicsApi
	);
}


ShaderProgram SetupContext::CreateShader(const std::string& name, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CreateShader(name, stages, macros);
}

ShaderProgram SetupContext::CompileShader(const std::string& code, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CompileShader(code, stages, macros);
}

gxapi::IPipelineState* SetupContext::CreatePSO(const gxapi::GraphicsPipelineStateDesc& desc) const {
	return m_graphicsApi->CreateGraphicsPipelineState(desc);
}

gxapi::IPipelineState* SetupContext::CreatePSO(const gxapi::ComputePipelineStateDesc& desc) const {
	return m_graphicsApi->CreateComputePipelineState(desc);
}


Binder SetupContext::CreateBinder(const std::vector<BindParameterDesc>& parameters, const std::vector<gxapi::StaticSamplerDesc>& staticSamplers) const {
	return Binder(m_graphicsApi, parameters, staticSamplers);
}



//------------------------------------------------------------------------------
// Render Context
//------------------------------------------------------------------------------


RenderContext::RenderContext(MemoryManager* memoryManager = nullptr,
							 CbvSrvUavHeap* srvHeap = nullptr,
							 VolatileViewHeap* volatileViewHeap = nullptr,
							 ShaderManager* shaderManager = nullptr,
							 gxapi::IGraphicsApi* graphicsApi = nullptr,
							 CommandAllocatorPool* commandAllocatorPool = nullptr,
							 ScratchSpacePool* scratchSpacePool = nullptr)
	: m_memoryManager(memoryManager),
	m_srvHeap(srvHeap),
	m_volatileViewHeap(volatileViewHeap),
	m_shaderManager(shaderManager),
	m_graphicsApi(graphicsApi),
	m_commandAllocatorPool(commandAllocatorPool),
	m_scratchSpacePool(scratchSpacePool)
{}


// Misc

VolatileConstBuffer RenderContext::CreateVolatileConstBuffer(const void* data, size_t size) const {
	VolatileConstBuffer result = m_memoryManager->CreateVolatileConstBuffer(data, (uint32_t)size);
	return result;
}

ConstBufferView RenderContext::CreateCbv(VolatileConstBuffer& buffer, size_t offset, size_t size) const {
	return ConstBufferView(
		buffer,
		m_volatileViewHeap->Allocate(),
		m_graphicsApi
	);
}

ShaderProgram RenderContext::CreateShader(const std::string& name, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CreateShader(name, stages, macros);
}

ShaderProgram RenderContext::CompileShader(const std::string& code, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CompileShader(code, stages, macros);
}

gxapi::IPipelineState* RenderContext::CreatePSO(const gxapi::GraphicsPipelineStateDesc& desc) const {
	return m_graphicsApi->CreateGraphicsPipelineState(desc);
}

gxapi::IPipelineState* RenderContext::CreatePSO(const gxapi::ComputePipelineStateDesc& desc) const {
	return m_graphicsApi->CreateComputePipelineState(desc);
}

Binder RenderContext::CreateBinder(const std::vector<BindParameterDesc>& parameters, const std::vector<gxapi::StaticSamplerDesc>& staticSamplers) const {
	return Binder(m_graphicsApi, parameters, staticSamplers);
}


// Query command list
GraphicsCommandList& RenderContext::AsGraphics() {
	if (!m_commandList) {
		m_commandList.reset(new GraphicsCommandList(m_graphicsApi, *m_commandAllocatorPool, *m_scratchSpacePool));
		return *dynamic_cast<GraphicsCommandList*>(m_commandList.get());
	}
	else if (m_type == gxapi::eCommandListType::GRAPHICS) {
		return *dynamic_cast<GraphicsCommandList*>(m_commandList.get());
	}
	else {
		throw std::logic_error("Your first call to AsType() determines the command list type. You did not choose GRAPHICS, thus this call is invalid.");
	}
}
ComputeCommandList& RenderContext::AsCompute() {
	if (!m_commandList) {
		m_commandList.reset(new ComputeCommandList(m_graphicsApi, *m_commandAllocatorPool, *m_scratchSpacePool));
		return *dynamic_cast<ComputeCommandList*>(m_commandList.get());
	}
	else if (m_type == gxapi::eCommandListType::COMPUTE) {
		return *dynamic_cast<ComputeCommandList*>(m_commandList.get());
	}
	else {
		throw std::logic_error("Your first call to AsType() determines the command list type. You did not choose COMPUTE, thus this call is invalid.");
	}
}
CopyCommandList& RenderContext::AsCopy() {
	if (!m_commandList) {
		m_commandList.reset(new CopyCommandList(m_graphicsApi, *m_commandAllocatorPool, *m_scratchSpacePool));
		return *dynamic_cast<CopyCommandList*>(m_commandList.get());
	}
	else if (m_type == gxapi::eCommandListType::COPY) {
		return *dynamic_cast<CopyCommandList*>(m_commandList.get());
	}
	else {
		throw std::logic_error("Your first call to AsType() determines the command list type. You did not choose COPY, thus this call is invalid.");
	}
}


} // namespace inl::gxeng