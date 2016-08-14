
#include "MemoryManager.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "GpuBuffer.hpp"

#include <algorithm>

namespace inl {
namespace gxeng {


MemoryManager::MemoryManager(gxapi::IGraphicsApi* graphicsApi, HighLevelDescHeap* heap) :
	m_graphicsApi(graphicsApi),
	m_descHeap(heap),
	m_criticalHeap(graphicsApi)
{}


void MemoryManager::LockResident(std::vector<GenericResource*> resources) {
	std::vector<gxapi::IResource*> underlying;
	underlying.reserve(resources.size());
	for (auto curr : resources) {
		underlying.push_back(curr->m_resource);
	}

	m_graphicsApi->MakeResident(underlying);
}


void MemoryManager::UnlockResident(std::vector<GenericResource*> resources) {
	std::vector<gxapi::IResource*> underlying;
	underlying.reserve(resources.size());
	for (auto curr : resources) {
		underlying.push_back(curr->m_resource);
	}

	//TODO do not evict the resource here! Only evict when space is needed.
	m_graphicsApi->Evict(underlying);
}


VertexBuffer* MemoryManager::CreateVertexBuffer(eResourceHeapType heap, size_t size) {
	std::unique_ptr<VertexBuffer> result(new VertexBuffer(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Buffer(size));
	return result.release();
}


IndexBuffer* MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size) {
	std::unique_ptr<IndexBuffer> result(new IndexBuffer(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Buffer(size));
	return result.release();
}


Texture1D* MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t elementCount) {
	if (elementCount < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	std::unique_ptr<Texture1D> result(new Texture1D(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Texture1DArray(width, format, elementCount));
	return result.release();
}


Texture2D* MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t elementCount) {
	if (elementCount < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	std::unique_ptr<Texture2D> result(new Texture2D(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Texture2DArray(width, height, format, elementCount));
	return result.release();
}


Texture3D* MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	std::unique_ptr<Texture3D> result(new Texture3D(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::Texture3D(width, height, depth, format));
	return result.release();
}


TextureCube* MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format) {
	std::unique_ptr<TextureCube> result(new TextureCube(m_descHeap->AllocateOnTextureSpace()));
	InitializeResource(heap, result.get(), gxapi::ResourceDesc::CubeMap(width, height, format));
	return result.release();
}


void MemoryManager::InitializeResource(eResourceHeapType heap, GenericResource* resource, const gxapi::ResourceDesc& desc) {
	
	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		resource->m_resource = m_criticalHeap.Allocate(resource, desc);
		break;
	}

	m_graphicsApi->CreateShaderResourceView(resource->m_resource, resource->m_resourceView.Get());
}


} // namespace gxeng
} // namespace inl
