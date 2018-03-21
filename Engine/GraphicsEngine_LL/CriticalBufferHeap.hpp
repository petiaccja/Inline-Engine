#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include "MemoryObject.hpp"
#include "BufferHeap.hpp"

namespace inl {
namespace gxeng {

namespace impl {

class CriticalBufferHeap : public BufferHeap {
public:
	CriticalBufferHeap(gxapi::IGraphicsApi* graphicsApi);

	VertexBuffer CreateVertexBuffer(size_t size) override;
	IndexBuffer CreateIndexBuffer(size_t size, size_t indexCount) override;
	Texture1D CreateTexture1D(const Texture1DDesc& desc, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE) override;
	Texture2D CreateTexture2D(const Texture2DDesc& desc, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE) override;
	Texture3D CreateTexture3D(const Texture3DDesc& desc, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE) override;

protected:
	using UniquePtr = std::unique_ptr<gxapi::IResource, std::function<void(const gxapi::IResource*)>>;
	UniquePtr Allocate(gxapi::ResourceDesc desc, gxapi::ClearValue* clearValue = nullptr);
	gxapi::ClearValue DetermineClearValue(const gxapi::ResourceDesc& desc);
private:
	gxapi::IGraphicsApi* m_graphicsApi;
};


} // namespace impl
} // namespace gxeng
} // namespace inl
