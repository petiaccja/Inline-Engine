
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


VolatileConstBuffer MemoryManager::CreateVolatileConstBuffer(void* data, size_t size) {
	return m_constBufferHeap.CreateVolatileBuffer(data, size);
}


PersistentConstBuffer MemoryManager::CreatePersistentConstBuffer(void * data, size_t size) {
	return m_constBufferHeap.CreatePersistentBuffer(data, size);
}


std::unique_ptr<VertexBuffer> MemoryManager::CreateVertexBuffer(eResourceHeapType heap, size_t size) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	std::unique_ptr<VertexBuffer> result(new VertexBuffer(params.resource));
	result->_SetResident(params.residency);
	return result;
}


std::unique_ptr<IndexBuffer> MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size, size_t indexCount) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	std::unique_ptr<IndexBuffer> result(new IndexBuffer(params.resource, indexCount));
	result->_SetResident(params.residency);
	return result;
}


std::unique_ptr<Texture1D> MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture1DArray(width, format, arraySize));

	std::unique_ptr<Texture1D> result(new Texture1D(params.resource));
	result->_SetResident(params.residency);
	return result;
}


std::unique_ptr<Texture2D> MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture2DArray(width, height, format, arraySize));

	std::unique_ptr<Texture2D> result(new Texture2D(params.resource));
	result->_SetResident(params.residency);
	return result;
}


std::unique_ptr<Texture3D> MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture3D(width, height, depth, format));

	std::unique_ptr<Texture3D> result(new Texture3D(params.resource));
	result->_SetResident(params.residency);
	return result;
}


std::unique_ptr<TextureCube> MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format) {
	impl::InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::CubeMap(width, height, format));

	std::unique_ptr<TextureCube> result(new TextureCube(params.resource));
	result->_SetResident(params.residency);
	return result;
}


impl::InitialResourceParameters MemoryManager::AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc& desc) {

	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		return m_criticalHeap.Allocate(desc);
		break;
	}

	assert(false);
	return impl::InitialResourceParameters();
}


} // namespace gxeng
} // namespace inl
