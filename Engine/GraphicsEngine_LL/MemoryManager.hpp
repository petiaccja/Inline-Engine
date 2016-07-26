#pragma once

#include "HighLevelDescHeap.hpp"
#include "GpuBuffer.hpp"
#include "ResourceHeap.hpp"

#include "../GraphicsApi_LL/Common.hpp"
#include "../GraphicsApi_D3D12/DescriptorHeap.hpp"
#include "../GraphicsApi_D3D12/GraphicsApi.hpp"

#include <iostream>

namespace inl {
namespace gxeng {


class MemoryManager {
public:
	MemoryManager(gxapi::IGraphicsApi* graphicsApi, HighLevelDescHeap* heap);

	void IWantToUseThisResourceRightNow(GenericResource*) = delete; //TODO
	void OkNowIDontNeedThisResourceForAWhile(GenericResource*) = delete; //TODO

	VertexBuffer* CreateVertexBuffer(eResourceHeapType heap, size_t size);
	IndexBuffer* CreateIndexBuffer(eResourceHeapType heap, size_t size);
	Texture1D* CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t elementCount = 1);
	Texture2D* CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t elementCount = 1);
	Texture3D* CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format);
	TextureCube* CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	HighLevelDescHeap* m_descHeap;
	impl::CriticalBufferHeap m_criticalHeap;

protected:
	void InitializeResource(eResourceHeapType heap, GenericResource* resource, const gxapi::ResourceDesc& desc);
};


} // namespace gxeng
} // namespace inl
