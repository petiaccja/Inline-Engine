
#include "MemoryManager.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "GpuBuffer.hpp"

#include <algorithm>
#include <cassert>


namespace inl {
namespace gxeng {


MemoryManager::MemoryManager(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi),
	m_criticalHeap(graphicsApi),
	m_uploadHeap(graphicsApi),
	m_constBufferHeap(graphicsApi)
{}


void MemoryManager::LockResident(const std::vector<GenericResource*>& resources) {
	LockResident(resources.begin(), resources.end());
}


void MemoryManager::UnlockResident(const std::vector<GenericResource*>& resources) {
	UnlockResident(resources.begin(), resources.end());
}


UploadHeap& MemoryManager::GetUploadHeap() {
	return m_uploadHeap;
}


VolatileConstBuffer MemoryManager::CreateVolatileConstBuffer(void* data, uint32_t size) {
	return m_constBufferHeap.CreateVolatileBuffer(data, size);
}


PersistentConstBuffer MemoryManager::CreatePersistentConstBuffer(void * data, uint32_t size) {
	return m_constBufferHeap.CreatePersistentBuffer(data, size);
}


VertexBuffer MemoryManager::CreateVertexBuffer(eResourceHeapType heap, size_t size) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	VertexBuffer result(params.resource);
	result._SetResident(params.residency);
	return result;
}


IndexBuffer MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size, size_t indexCount) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	IndexBuffer result(params.resource, indexCount);
	result._SetResident(params.residency);
	return result;
}


Texture1D MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, gxapi::eResourceFlags flags, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture1DArray(width, format, arraySize, flags));

	Texture1D result(params.resource);
	result._SetResident(params.residency);
	return result;
}


Texture2D MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, gxapi::eResourceFlags flags, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture2DArray(width, height, format, arraySize, flags));

	Texture2D result(params.resource);
	result._SetResident(params.residency);
	return result;
}


Texture3D MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format, gxapi::eResourceFlags flags) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture3D(width, height, depth, format));

	Texture3D result(params.resource);
	result._SetResident(params.residency);
	return result;
}


TextureCube MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, gxapi::eResourceFlags flags) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::CubeMap(width, height, format));

	TextureCube result(params.resource);
	result._SetResident(params.residency);
	return result;
}


impl::InitialResourceParameters MemoryManager::AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc& desc) {

	gxapi::ClearValue* pClearValue = nullptr;
	gxapi::ClearValue clearValue(desc.textureDesc.format, 1, 0);
	if (desc.type == gxapi::eResourceType::TEXTURE && desc.textureDesc.flags & gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL) {
		pClearValue = &clearValue;
	}

	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		return m_criticalHeap.Allocate(desc, pClearValue);
		break;
	}

	assert(false);
	return impl::InitialResourceParameters();
}


} // namespace gxeng
} // namespace inl
