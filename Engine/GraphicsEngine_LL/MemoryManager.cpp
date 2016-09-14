
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
	InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));
	VertexBuffer* result = new VertexBuffer(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


IndexBuffer* MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size) {
	InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));
	IndexBuffer* result = new IndexBuffer(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


Texture1D* MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture1DArray(width, format, arraySize));
	Texture1D* result = new Texture1D(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


Texture2D* MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture2DArray(width, height, format, arraySize));

	Texture2D* result = new Texture2D(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


Texture3D* MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::Texture3D(width, height, depth, format));

	Texture3D* result = new Texture3D(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


TextureCube* MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format) {
	InitialResourceParameters params = AllocateResource(heap, gxapi::ResourceDesc::CubeMap(width, height, format));

	TextureCube* result = new TextureCube(std::move(params.desc), params.resource);
	result->_SetResident(params.residency);
	return result;
}


MemoryManager::InitialResourceParameters MemoryManager::AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc & desc) {
	InitialResourceParameters retval(m_descHeap->AllocateOnTextureSpace());

	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		retval.resource = m_criticalHeap.Allocate(desc);
		retval.residency = true;
		break;
	}

	return retval;
}


} // namespace gxeng
} // namespace inl
