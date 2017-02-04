#pragma once

#include "MemoryObject.hpp"
#include "ResourceView.hpp"
#include "ShaderManager.hpp"
#include "VolatileViewHeap.hpp"
#include <cstdint>


namespace inl {
namespace gxeng {

class MemoryManager;
class CbvSrvUavHeap;
class RTVHeap;
class DSVHeap;

class GraphicsContext {
public:
	GraphicsContext(MemoryManager* memoryManager = nullptr,
					CbvSrvUavHeap* srvHeap = nullptr,
					RTVHeap* rtvHeap = nullptr,
					DSVHeap* dsvHeap = nullptr,
					int processorCount = 0,
					int deviceCount = 0,
					ShaderManager* shaderManager = nullptr,
					gxapi::IGraphicsApi* graphicsApi = nullptr);
	GraphicsContext(const GraphicsContext& rhs) = default;
	GraphicsContext(GraphicsContext&& rhs) = default;
	GraphicsContext& operator=(const GraphicsContext& rhs) = default;
	GraphicsContext& operator=(GraphicsContext&& rhs) = default;

	// Parallelism
	int GetProcessorCoreCount() const;
	int GetGraphicsDeviceCount() const;

	// Create pipeline textures
	Texture2D CreateTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize = 1) const;
	Texture2D CreateRenderTarget2D(uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize = 1) const;
	Texture2D CreateDepthStencil2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool shaderResource, uint16_t arraySize = 1) const;
	Texture2D CreateRWTexture2D(uint64_t width, uint32_t height, gxapi::eFormat format, bool renderTarget, uint16_t arraySize = 1) const;
	TextureView2D CreateSrv(Texture2D& texture, gxapi::eFormat format, gxapi::SrvTexture2DArray desc = {}) const;
	RenderTargetView2D CreateRtv(Texture2D& renderTarget, gxapi::eFormat format, gxapi::RtvTexture2DArray desc) const;
	DepthStencilView2D CreateDsv(Texture2D& depthStencilView, gxapi::eFormat format, gxapi::DsvTexture2DArray desc) const;
	RWTextureView2D CreateUav(Texture2D& rwTexture, gxapi::eFormat format, gxapi::UavTexture2DArray desc) const;

	// Vertex buffer
	VertexBuffer CreateVertexBuffer(const void* data, size_t size);
	IndexBuffer CreateIndexBuffer(const void* data, size_t size, size_t indexCount);

	// Constant buffers
	VolatileConstBuffer CreateVolatileConstBuffer(const void* data, size_t size);
	ConstBufferView CreateCbv(VolatileConstBuffer& buffer, size_t offset, size_t size, VolatileViewHeap& viewHeap);

	// Shaders and PSOs
	ShaderProgram CreateShader(const std::string& name, ShaderParts stages, const std::string& macros);
	gxapi::IPipelineState* CreatePSO(const gxapi::GraphicsPipelineStateDesc& desc);
	gxapi::IPipelineState* CreatePSO(const gxapi::ComputePipelineStateDesc& desc);

private:
	// Memory management stuff
	MemoryManager* m_memoryManager;
	CbvSrvUavHeap* m_srvHeap;
	RTVHeap* m_rtvHeap;
	DSVHeap* m_dsvHeap;
	int m_processorCount;
	int m_deviceCount;

	// Shaders and PSOs
	ShaderManager* m_shaderManager;
	gxapi::IGraphicsApi* m_graphicsApi;
};



} // namespace gxeng
} // namespace inl

