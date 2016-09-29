
#include "MemoryManager.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "GpuBuffer.hpp"

#include <algorithm>
#include <cassert>

namespace inl {
namespace gxeng {


MemoryManager::MemoryManager(gxapi::IGraphicsApi* graphicsApi, HighLevelDescHeap* heap) :
	m_graphicsApi(graphicsApi),
	m_descHeap(heap),
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


ConstBuffer MemoryManager::CreateConstBuffer(void* data, size_t size) {
	return m_constBufferHeap.CreateBuffer(m_descHeap->AllocateOnTextureSpace(), data, size);
}


VertexBuffer* MemoryManager::CreateVertexBuffer(eResourceHeapType heap, size_t size) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	VertexBuffer* result = new VertexBuffer(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


IndexBuffer* MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	IndexBuffer* result = new IndexBuffer(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


Texture1D* MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture1DArray(width, format, arraySize));

	Texture1D* result = new Texture1D(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


Texture2D* MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture2DArray(width, height, format, arraySize));

	Texture2D* result = new Texture2D(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


Texture3D* MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture3D(width, height, depth, format));

	Texture3D* result = new Texture3D(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


TextureCube* MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::CubeMap(width, height, format));

	TextureCube* result = new TextureCube(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


impl::InitialResourceParameters MemoryManager::AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc& desc) {

	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		return m_criticalHeap.Allocate(m_descHeap->AllocateOnTextureSpace(), desc);
		break;
	}

	assert(false);
	return impl::InitialResourceParameters(DescriptorReference(gxapi::DescriptorHandle(), nullptr));
}


} // namespace gxeng
} // namespace inl
