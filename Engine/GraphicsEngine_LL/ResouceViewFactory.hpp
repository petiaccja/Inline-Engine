#pragma once

#include "ResourceView.hpp"
#include "HighLevelDescHeap.hpp"

namespace inl {
namespace gxeng {

class PersistentConstBuffer;
class VolatileConstBuffer;
class Texture2D;
class VertexBuffer;
class LinearBuffer;

class ResourceViewFactory {
public:
	ResourceViewFactory(gxapi::IGraphicsApi* graphicsApi);

	ConstBufferView CreateConstBufferView(const std::shared_ptr<PersistentConstBuffer>& resource);
	ConstBufferView CreateConstBufferView(const std::shared_ptr<VolatileConstBuffer>& resource, ScratchSpace* scratchSpace);
	DepthStencilView CreateDepthStencilView(const std::shared_ptr<Texture2D>& resource, gxapi::DsvTexture2DArray desc);
	RenderTargetView CreateRenderTargetView(const std::shared_ptr<Texture2D>& resource);
	RenderTargetView CreateRenderTargetView(const std::shared_ptr<Texture2D>& resource, gxapi::RtvTexture2DArray desc);
	VertexBufferView CreateVertexBufferView(const std::shared_ptr<VertexBuffer>& resource, uint32_t stride, uint32_t size);
	BufferSRV CreateBufferSRV(const std::shared_ptr<LinearBuffer>& resource, gxapi::eFormat format, gxapi::SrvBuffer desc);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	HostDescHeap m_CBV_SRV_UAV_Heap;
	HostDescHeap m_DSV_Heap;
	HostDescHeap m_RTV_Heap;
};


} // namespace gxeng
} // namespace inl


