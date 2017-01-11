#include "GraphicsContext.hpp"
#include "MemoryManager.hpp"


namespace inl {
namespace gxeng {


GraphicsContext::GraphicsContext(MemoryManager* memoryManager,
	PersistentResViewHeap* srvHeap,
	RTVHeap* rtvHeap,
	DSVHeap* dsvHeap,
	int processorCount,
	int deviceCount,
	ShaderManager* shaderManager,
	gxapi::IGraphicsApi* graphicsApi)

	: m_memoryManager(memoryManager),
	m_srvHeap(srvHeap),
	m_rtvHeap(rtvHeap),
	m_dsvHeap(dsvHeap),
	m_processorCount(processorCount),
	m_deviceCount(deviceCount),
	m_shaderManager(shaderManager),
	m_graphicsApi(graphicsApi)
{}



int GraphicsContext::GetProcessorCoreCount() const {
	return m_processorCount;
}
int GraphicsContext::GetGraphicsDeviceCount() const {
	return m_deviceCount;
}


Texture2D GraphicsContext::CreateTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format, gxapi::eResourceFlags::NONE, arraySize);
	return texture;
}


Texture2D GraphicsContext::CreateRenderTarget2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool shaderResource, uint16_t arraySize) const {
	if (m_memoryManager == nullptr) throw std::logic_error("Cannot create texture without memory manager.");

	gxapi::eResourceFlags flags = gxapi::eResourceFlags::ALLOW_RENDER_TARGET;
	if (!shaderResource) {
		flags += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
	}
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


Texture2DSRV GraphicsContext::CreateSrv(Texture2D& texture, gxapi::eFormat format, gxapi::SrvTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create src without srv heap.");

	return Texture2DSRV{texture, *m_srvHeap, format, desc };
}


RenderTargetView GraphicsContext::CreateRtv(Texture2D& texture, gxapi::eFormat format, gxapi::RtvTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create rtv without rtv heap.");

	return RenderTargetView{ texture, *m_rtvHeap, format, desc };
}


DepthStencilView GraphicsContext::CreateDsv(Texture2D& texture, gxapi::eFormat format, gxapi::DsvTexture2DArray desc) const {
	if (m_srvHeap == nullptr) throw std::logic_error("Cannot create dsv without dsv heap.");

	return DepthStencilView{ texture, *m_dsvHeap, format, desc };
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


ShaderProgram GraphicsContext::CreateShader(const std::string& name, ShaderParts stages, const std::string& macros) {
	return m_shaderManager->CreateShader(name, stages, macros);
}

gxapi::IPipelineState* GraphicsContext::CreatePSO(const gxapi::GraphicsPipelineStateDesc& desc) {
	return m_graphicsApi->CreateGraphicsPipelineState(desc);
}



} // namespace gxeng
} // namespace inl

