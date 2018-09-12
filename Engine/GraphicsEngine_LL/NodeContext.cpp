#include "NodeContext.hpp"

#include "MemoryManager.hpp" 
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"
#include "GraphicsCommandList.hpp"


namespace inl::gxeng {



//------------------------------------------------------------------------------
// Engine Context
//------------------------------------------------------------------------------

EngineContext::EngineContext(int cpuCount, int gpuCount) {
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

SetupContext::SetupContext(MemoryManager* memoryManager,
						   CbvSrvUavHeap* srvHeap,
						   RTVHeap* rtvHeap,
						   DSVHeap* dsvHeap,
						   ShaderManager* shaderManager,
						   gxapi::IGraphicsApi* graphicsApi)
	: m_memoryManager(memoryManager),
	m_srvHeap(srvHeap),
	m_rtvHeap(rtvHeap),
	m_dsvHeap(dsvHeap),
	m_shaderManager(shaderManager),
	m_graphicsApi(graphicsApi)
{}


Texture2D SetupContext::CreateTexture2D(const Texture2DDesc& desc, const TextureUsage& usage) const {
	gxapi::eResourceFlags flags;

	if (!usage.shaderResource) flags += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
	if (usage.renderTarget) flags += gxapi::eResourceFlags::ALLOW_RENDER_TARGET;
	if (usage.depthStencil) flags += gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL;
	if (usage.randomAccess) flags += gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS;

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeap::CRITICAL, desc, flags);
	return texture;
}

Texture3D SetupContext::CreateTexture3D(const Texture3DDesc& desc, const TextureUsage& usage) const {
	gxapi::eResourceFlags flags;

	if (!usage.shaderResource) flags += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
	if (usage.renderTarget) flags += gxapi::eResourceFlags::ALLOW_RENDER_TARGET;
	if (usage.depthStencil) flags += gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL;
	if (usage.randomAccess) flags += gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS;

	Texture3D texture = m_memoryManager->CreateTexture3D(eResourceHeap::CRITICAL, desc, flags);
	return texture;
}


TextureView2D SetupContext::CreateSrv(const Texture2D& texture, gxapi::eFormat format, gxapi::SrvTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw InvalidStateException("Cannot create srv without srv/cbv/uav heap.");

	return TextureView2D{ texture, *m_srvHeap, format, desc };
}

TextureViewCube SetupContext::CreateSrv(const Texture2D & texture, gxapi::eFormat format, gxapi::SrvTextureCubeArray desc) const {
	if (m_srvHeap == nullptr) throw InvalidStateException("Cannot create srv without srv/cbv/uav heap.");

	return TextureViewCube{ texture, *m_srvHeap, format, desc };
}

TextureView3D SetupContext::CreateSrv(const Texture3D & texture, gxapi::eFormat format, gxapi::SrvTexture3D desc) const {
	if (m_srvHeap == nullptr) throw InvalidStateException("Cannot create srv without srv/cbv/uav heap.");

	return TextureView3D{ texture, *m_srvHeap, format, desc };
}

RenderTargetView2D SetupContext::CreateRtv(const Texture2D& texture, gxapi::eFormat format, gxapi::RtvTexture2DArray desc) const {
	if (m_rtvHeap == nullptr) throw InvalidStateException("Cannot create rtv without rtv heap.");

	return RenderTargetView2D{ texture, *m_rtvHeap, format, desc };
}


DepthStencilView2D SetupContext::CreateDsv(const Texture2D& texture, gxapi::eFormat format, gxapi::DsvTexture2DArray desc) const {
	if (m_dsvHeap == nullptr) throw InvalidStateException("Cannot create dsv without dsv heap.");

	return DepthStencilView2D{ texture, *m_dsvHeap, format, desc };
}


RWTextureView2D SetupContext::CreateUav(const Texture2D& rwTexture, gxapi::eFormat format, gxapi::UavTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw InvalidStateException("Cannot create uav wihtout srv/cbv/uav heap.");

	return RWTextureView2D{ rwTexture, *m_srvHeap, format, desc };
}

RWTextureView3D SetupContext::CreateUav(const Texture3D& rwTexture, gxapi::eFormat format, gxapi::UavTexture3D desc) const {
	if (m_srvHeap == nullptr) throw InvalidStateException("Cannot create uav wihtout srv/cbv/uav heap.");

	return RWTextureView3D{ rwTexture, *m_srvHeap, format, desc };
}


VertexBuffer SetupContext::CreateVertexBuffer(size_t size) const {
	VertexBuffer result = m_memoryManager->CreateVertexBuffer(eResourceHeap::CRITICAL, size);
	return result;
}


IndexBuffer SetupContext::CreateIndexBuffer(size_t size, size_t indexCount) const {
	IndexBuffer result = m_memoryManager->CreateIndexBuffer(eResourceHeap::CRITICAL, size, indexCount);
	return result;
}

ShaderProgram SetupContext::CreateShader(const std::string& name, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CreateShader(name, stages, macros);
}

ShaderProgram SetupContext::CompileShader(const std::string& code, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CompileShader(code, stages, macros);
}

std::string SetupContext::LoadShader(const std::string& name) const {
	return m_shaderManager->LoadShaderSource(name);
}

void SetupContext::StoreShader(const std::string& name, const std::string& code) const {
	m_shaderManager->AddSourceCode(name, code);
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


RenderContext::RenderContext(MemoryManager* memoryManager,
							 CbvSrvUavHeap* srvHeap,
							 ShaderManager* shaderManager,
							 gxapi::IGraphicsApi* graphicsApi,
							 CommandListPool* commandListPool,
							 CommandAllocatorPool* commandAllocatorPool,
							 ScratchSpacePool* scratchSpacePool,
							 std::unique_ptr<BasicCommandList> inheritedList,
							 std::unique_ptr<VolatileViewHeap> inheritedVheap)
	: m_memoryManager(memoryManager),
	m_srvHeap(srvHeap),
	m_shaderManager(shaderManager),
	m_graphicsApi(graphicsApi),
	m_commandListPool(commandListPool),
	m_commandAllocatorPool(commandAllocatorPool),
	m_scratchSpacePool(scratchSpacePool),
	m_inheritedCommandList(std::move(inheritedList)),
	m_vheap(std::move(inheritedVheap))
{}


// Misc

VolatileConstBuffer RenderContext::CreateVolatileConstBuffer(const void* data, size_t size) const {
	VolatileConstBuffer result = m_memoryManager->CreateVolatileConstBuffer(data, (uint32_t)size);
	return result;
}

ConstBufferView RenderContext::CreateCbv(VolatileConstBuffer& buffer, size_t offset, size_t size) const {
	InitVheap();
	return ConstBufferView(
		buffer,
		m_vheap->Allocate(),
		m_graphicsApi
	);
}

ShaderProgram RenderContext::CreateShader(const std::string& name, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CreateShader(name, stages, macros);
}

ShaderProgram RenderContext::CompileShader(const std::string& code, ShaderParts stages, const std::string& macros) const {
	return m_shaderManager->CompileShader(code, stages, macros);
}

std::string RenderContext::LoadShader(const std::string& name) const {
	return m_shaderManager->LoadShaderSource(name);
}
void RenderContext::StoreShader(const std::string& name, const std::string& code) const {
	m_shaderManager->AddSourceCode(name, code);
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


void RenderContext::Upload(const LinearBuffer& target,
						   size_t offset,
						   const void* data,
						   size_t size)
{
	if (!m_commandList) {
		throw InvalidCallException("Call one of AsGraphics/AsCompute/AsCopy before calling this.");
	}
	CopyCommandList& commandList = *dynamic_cast<CopyCommandList*>(m_commandList.get());
	m_memoryManager->GetUploadManager().UploadNow(commandList, target, offset, data, size);
}

void RenderContext::Upload(const Texture2D& target,
						   uint32_t offsetX,
						   uint32_t offsetY,
						   uint32_t subresource,
						   const void* data,
						   uint64_t width,
						   uint32_t height,
						   gxapi::eFormat format,
						   size_t bytesPerRow)
{
	if (!m_commandList) {
		throw InvalidCallException("Call one of AsGraphics/AsCompute/AsCopy before calling this.");
	}
	CopyCommandList& commandList = *dynamic_cast<CopyCommandList*>(m_commandList.get());
	m_memoryManager->GetUploadManager().UploadNow(commandList, target, offsetX, offsetY, subresource, data, width, height, format, bytesPerRow);
}

// Query command list
GraphicsCommandList& RenderContext::AsGraphics() {
	InitVheap();
	if (!m_commandList) {
		if (m_inheritedCommandList && m_inheritedCommandList->GetType() == gxapi::eCommandListType::GRAPHICS) {
			m_commandList = std::move(m_inheritedCommandList);
		}
		else {
			m_commandList.reset(new GraphicsCommandList(m_graphicsApi, *m_commandListPool, *m_commandAllocatorPool, *m_scratchSpacePool, *m_memoryManager, *m_vheap.get()));
		}
		m_commandList->BeginDebuggerEvent(m_TMP_commandListName); // TMP
		m_commandList->SetName(m_TMP_commandListName);
		m_type = gxapi::eCommandListType::GRAPHICS;
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
	InitVheap();
	if (!m_commandList) {
		if (m_inheritedCommandList && m_inheritedCommandList->GetType() == gxapi::eCommandListType::COMPUTE) {
			m_commandList = std::move(m_inheritedCommandList);
		}
		else {
			m_commandList.reset(new GraphicsCommandList(m_graphicsApi, *m_commandListPool, *m_commandAllocatorPool, *m_scratchSpacePool, *m_memoryManager, *m_vheap.get())); // only graphics queues now
		}
		m_commandList->BeginDebuggerEvent(m_TMP_commandListName); // TMP
		m_type = gxapi::eCommandListType::COMPUTE;
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
	InitVheap();
	if (!m_commandList) {
		if (m_inheritedCommandList && m_inheritedCommandList->GetType() == gxapi::eCommandListType::COPY) {
			m_commandList = std::move(m_inheritedCommandList);
		}
		else {
			m_commandList.reset(new GraphicsCommandList(m_graphicsApi, *m_commandListPool, *m_commandAllocatorPool, *m_scratchSpacePool, *m_memoryManager, *m_vheap.get())); // only graphics queues now
		}
		m_commandList->BeginDebuggerEvent(m_TMP_commandListName); // TMP
		m_type = gxapi::eCommandListType::COPY;
		return *dynamic_cast<CopyCommandList*>(m_commandList.get());
	}
	else if (m_type == gxapi::eCommandListType::COPY) {
		return *dynamic_cast<CopyCommandList*>(m_commandList.get());
	}
	else {
		throw std::logic_error("Your first call to AsType() determines the command list type. You did not choose COPY, thus this call is invalid.");
	}
}

void RenderContext::Decompose(std::unique_ptr<BasicCommandList>& inheritedList,
							  std::unique_ptr<BasicCommandList>& currentList, 
							  std::unique_ptr<VolatileViewHeap>& currentVheap) 
{
	inheritedList = std::move(m_inheritedCommandList);
	currentList = std::move(m_commandList);
	currentVheap = std::move(m_vheap);
}

void RenderContext::InitVheap() const {
	if (!m_vheap) {
		m_vheap = std::make_unique<VolatileViewHeap>(m_graphicsApi);
	}
}


} // namespace inl::gxeng