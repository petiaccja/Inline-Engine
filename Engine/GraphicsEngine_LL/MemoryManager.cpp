
#include "MemoryManager.hpp"
#include "MemoryObject.hpp"

#include <GraphicsApi_LL/Exception.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/Common.hpp>
#include <BaseLibrary/Exception/Exception.hpp>

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


void MemoryManager::LockResident(const std::vector<MemoryObject>& resources) {
	// Lock resident wants to modify contents but not the vector.
	auto& nonconst = const_cast<std::vector<MemoryObject>&>(resources);
	LockResident(nonconst.begin(), nonconst.end());
}


void MemoryManager::UnlockResident(const std::vector<MemoryObject>& resources) {
	auto& nonconst = const_cast<std::vector<MemoryObject>&>(resources);
	UnlockResident(nonconst.begin(), nonconst.end());
}


UploadManager& MemoryManager::GetUploadManager() {
	return m_uploadHeap;
}

ConstantBufferHeap & MemoryManager::GetConstBufferHeap() {
	return m_constBufferHeap;
}


VolatileConstBuffer MemoryManager::CreateVolatileConstBuffer(const void* data, uint32_t size) {
	return m_constBufferHeap.CreateVolatileConstBuffer(data, size);
}


PersistentConstBuffer MemoryManager::CreatePersistentConstBuffer(const void * data, uint32_t size) {
	return m_constBufferHeap.CreatePersistentConstBuffer(data, size);
}


VertexBuffer MemoryManager::CreateVertexBuffer(eResourceHeap heap, size_t size) {
	return GetHeap(heap).CreateVertexBuffer(size);
}


IndexBuffer MemoryManager::CreateIndexBuffer(eResourceHeap heap, size_t size, size_t indexCount) {
	return GetHeap(heap).CreateIndexBuffer(size, indexCount);
}


Texture1D MemoryManager::CreateTexture1D(eResourceHeap heap, const Texture1DDesc& desc, gxapi::eResourceFlags flags) {
	return GetHeap(heap).CreateTexture1D(desc, flags);
}


Texture2D MemoryManager::CreateTexture2D(eResourceHeap heap, const Texture2DDesc& desc, gxapi::eResourceFlags flags) {
	return GetHeap(heap).CreateTexture2D(desc, flags);
}


Texture3D MemoryManager::CreateTexture3D(eResourceHeap heap, const Texture3DDesc& desc, gxapi::eResourceFlags flags) {
	return GetHeap(heap).CreateTexture3D(desc, flags);
}


BufferHeap& MemoryManager::GetHeap(eResourceHeap heap) {
	switch (heap) {
		case eResourceHeap::UPLOAD: throw NotImplementedException("Memory heap not implemented yet.");
		case eResourceHeap::CONSTANT: return m_constBufferHeap;
		case eResourceHeap::STREAMING: throw NotImplementedException("Memory heap not implemented yet.");
		case eResourceHeap::PIPELINE: throw NotImplementedException("Memory heap not implemented yet.");
		case eResourceHeap::CRITICAL: return m_criticalHeap;
		case eResourceHeap::INVALID: throw InvalidArgumentException("Invalid memory heap.");
		default: throw NotImplementedException("Memory heap not implemented yet.");
	}
}


} // namespace gxeng
} // namespace inl
