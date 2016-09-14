
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
	m_criticalHeap(graphicsApi)
{}


void MemoryManager::LockResident(const std::vector<GenericResource*>& resources) {
	LockResident(resources.begin(), resources.end());
}


void MemoryManager::UnlockResident(const std::vector<GenericResource*>& resources) {
	UnlockResident(resources.begin(), resources.end());
}


VertexBuffer* MemoryManager::CreateVertexBuffer(eResourceHeapType heap, size_t size) {
	std::unique_ptr<VertexBuffer> result(new VertexBuffer(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Buffer(size));
	result->InitResourceStates(1, gxapi::eResourceState::COMMON);
	return result.release();
}


IndexBuffer* MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size) {
	std::unique_ptr<IndexBuffer> result(new IndexBuffer(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Buffer(size));
	result->InitResourceStates(1, gxapi::eResourceState::COMMON);
	return result.release();
}


Texture1D* MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	std::unique_ptr<Texture1D> result(new Texture1D(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Texture1DArray(width, format, arraySize));
	result->InitResourceStates(arraySize, gxapi::eResourceState::COMMON);
	return result.release();
}


Texture2D* MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	std::unique_ptr<Texture2D> result(new Texture2D(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Texture2DArray(width, height, format, arraySize));
	result->InitResourceStates(arraySize, gxapi::eResourceState::COMMON);
	return result.release();
}


Texture3D* MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	std::unique_ptr<Texture3D> result(new Texture3D(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Texture3D(width, height, depth, format));
	result->InitResourceStates(1, gxapi::eResourceState::COMMON);
	return result.release();
}


TextureCube* MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format) {
	std::unique_ptr<TextureCube> result(new TextureCube(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::CubeMap(width, height, format));
	result->InitResourceStates(6, gxapi::eResourceState::COMMON);
	return result.release();
}


void MemoryManager::InitializeResource(eResourceHeapType heap, GenericResource* resource, const gxapi::ResourceDesc& desc) {
	
	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		resource->m_resource = m_criticalHeap.Allocate(resource, desc);
		resource->m_deleter = std::bind(&impl::CriticalBufferHeap::ReleaseUnderlying, &m_criticalHeap, std::placeholders::_1);
		resource->m_resident = true;
		break;
	}

	m_graphicsApi->CreateShaderResourceView(resource->m_resource, resource->m_resourceView.Get());
}


} // namespace gxeng
} // namespace inl
