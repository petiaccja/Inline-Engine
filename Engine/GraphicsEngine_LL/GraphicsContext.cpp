#include "GraphicsContext.hpp"
#include "MemoryManager.hpp"

#include <GraphicsApi_LL/ISwapChain.hpp>

namespace inl {
namespace gxeng {


GraphicsContext::GraphicsContext(MemoryManager* memoryManager,
								 CbvSrvUavHeap* srvHeap,
								 RTVHeap* rtvHeap,
								 DSVHeap* dsvHeap,
								 int processorCount,
								 int deviceCount,
								 ShaderManager* shaderManager,
								 gxapi::ISwapChain* swapChain,
								 gxapi::IGraphicsApi* graphicsApi)

	: m_memoryManager(memoryManager),
	m_srvHeap(srvHeap),
	m_rtvHeap(rtvHeap),
	m_dsvHeap(dsvHeap),
	m_processorCount(processorCount),
	m_deviceCount(deviceCount),
	m_shaderManager(shaderManager),
	m_swapChain(swapChain),
	m_graphicsApi(graphicsApi)
{}



int GraphicsContext::GetProcessorCoreCount() const {
	return m_processorCount;
}
int GraphicsContext::GetGraphicsDeviceCount() const {
	return m_deviceCount;
}


gxapi::SwapChainDesc GraphicsContext::GetSwapChainDesc() const {
	return m_swapChain->GetDesc();
}


Texture2D GraphicsContext::CreateTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, gxapi::eResourceFlags::NONE, arraySize);
	return texture;
}


Texture2D GraphicsContext::CreateRenderTarget2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_RENDER_TARGET;

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, flags, arraySize);
	return texture;
}


Texture2D GraphicsContext::CreateDepthStencil2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool shaderResource, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL;
	if (!shaderResource) {
		flags += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
	}
	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, flags, arraySize);
	return texture;
}

Texture2D GraphicsContext::CreateRWTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool renderTarget, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS;
	if (renderTarget) { flags += gxapi::eResourceFlags::ALLOW_RENDER_TARGET; }

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, flags, arraySize);
	return texture;
}


TextureView2D GraphicsContext::CreateSrv(Texture2D& texture, gxapi::eFormat format, gxapi::SrvTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create srv without srv/cbv/uav heap.");

	return TextureView2D{ texture, *m_srvHeap, format, desc };
}


RenderTargetView2D GraphicsContext::CreateRtv(Texture2D& texture, gxapi::eFormat format, gxapi::RtvTexture2DArray desc) const {
	if (m_rtvHeap == nullptr) throw std::logic_error("Cannot create rtv without rtv heap.");

	return RenderTargetView2D{ texture, *m_rtvHeap, format, desc };
}


DepthStencilView2D GraphicsContext::CreateDsv(Texture2D& texture, gxapi::eFormat format, gxapi::DsvTexture2DArray desc) const {
	if (m_dsvHeap == nullptr) throw std::logic_error("Cannot create dsv without dsv heap.");

	return DepthStencilView2D{ texture, *m_dsvHeap, format, desc };
}


RWTextureView2D GraphicsContext::CreateUav(Texture2D& rwTexture, gxapi::eFormat format, gxapi::UavTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create uav wihtout srv/cbv/uav heap.");

	return RWTextureView2D{ rwTexture, *m_srvHeap, format, desc };
}


VertexBuffer GraphicsContext::CreateVertexBuffer(const void* data, size_t size) {
	VertexBuffer result = m_memoryManager->CreateVertexBuffer(eResourceHeapType::CRITICAL, size);
	m_memoryManager->GetUploadManager().Upload(result, 0, data, size);
	return result;
}


IndexBuffer GraphicsContext::CreateIndexBuffer(const void* data, size_t size, size_t indexCount) {
	IndexBuffer result = m_memoryManager->CreateIndexBuffer(eResourceHeapType::CRITICAL, size, indexCount);
	m_memoryManager->GetUploadManager().Upload(result, 0, data, size);
	return result;
}


VolatileConstBuffer GraphicsContext::CreateVolatileConstBuffer(const void * data, size_t size) {
	VolatileConstBuffer result = m_memoryManager->CreateVolatileConstBuffer(data, size);
	return result;
}


ConstBufferView GraphicsContext::CreateCbv(VolatileConstBuffer& buffer, size_t offset, size_t size, VolatileViewHeap& viewHeap) {
	return ConstBufferView(
		buffer,
		viewHeap.Allocate(),
		m_graphicsApi
	);
}


ShaderProgram GraphicsContext::CreateShader(const std::string& name, ShaderParts stages, const std::string& macros) {
	return m_shaderManager->CreateShader(name, stages, macros);
}

gxapi::IPipelineState* GraphicsContext::CreatePSO(const gxapi::GraphicsPipelineStateDesc& desc) {
	return m_graphicsApi->CreateGraphicsPipelineState(desc);
}

gxapi::IPipelineState* GraphicsContext::CreatePSO(const gxapi::ComputePipelineStateDesc& desc) {
	return m_graphicsApi->CreateComputePipelineState(desc);
}


} // namespace gxeng
} // namespace inl

