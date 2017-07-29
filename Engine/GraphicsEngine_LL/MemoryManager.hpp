#pragma once

#include "HostDescHeap.hpp"
#include "MemoryObject.hpp"
#include "CriticalBufferHeap.hpp"
#include "UploadManager.hpp"
#include "ConstBufferHeap.hpp"

#include "../GraphicsApi_LL/Common.hpp"
#include "../GraphicsApi_D3D12/DescriptorHeap.hpp"
#include "../GraphicsApi_D3D12/GraphicsApi.hpp"

#include <iostream>
#include <unordered_set>
#include <mutex>
#include <cassert>
#include <type_traits>

namespace inl {
namespace gxeng {

enum class eResourceHeapType { CRITICAL };

class MemoryManager {
public:
	MemoryManager(gxapi::IGraphicsApi* graphicsApi);

	/// <summary>
	/// Makes given resources resident.
	/// </summary>
	/// <exception cref="inl::gxapi::OutOfMemory">
	/// If there is not enough free memory in the resource's appropriate
	/// memory pool for the resource to fit in.
	/// </exception>
	void LockResident(const std::vector<MemoryObject>& resources);
	template<typename IterT>
	void LockResident(IterT begin, IterT end);

	/// <summary>
	/// WRITE A DESCRIPTION
	/// </summary>
	void UnlockResident(const std::vector<MemoryObject>& resources);
	template<typename IterT>
	void UnlockResident(IterT begin, IterT end);

	UploadManager& GetUploadManager();
	VolatileConstBuffer CreateVolatileConstBuffer(const void* data, uint32_t size);
	PersistentConstBuffer CreatePersistentConstBuffer(const void* data, uint32_t size);

	VertexBuffer CreateVertexBuffer(eResourceHeapType heap, size_t size);
	IndexBuffer CreateIndexBuffer(eResourceHeapType heap, size_t size, size_t indexCount);
	Texture1D CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE, uint16_t arraySize = 1);
	Texture2D CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE, uint16_t arraySize = 1);
	Texture3D CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE);
	//TextureCube CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE, uint16_t arraySize = 1);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	impl::CriticalBufferHeap m_criticalHeap;

	UploadManager m_uploadHeap;
	ConstantBufferHeap m_constBufferHeap;

	std::mutex m_evictablesMtx;
	std::unordered_set<MemoryObject> m_evictables;

protected:
	MemoryObjDesc AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc& desc);
};


template<typename IterT>
void MemoryManager::LockResident(IterT begin, IterT end) {
	static_assert(std::is_same<typename IterT::value_type, MemoryObject>::value);

	std::vector<gxapi::IResource*> lowLevelTargets;
	std::vector<MemoryObject> highLevelTargets;

	{
		std::lock_guard<std::mutex> lock(m_evictablesMtx);

		IterT currIter = begin;
		while (currIter != end) {
			auto& curr = *(currIter++);
			auto currEvictableIter = m_evictables.find(curr);
			bool evictable = currEvictableIter != m_evictables.end();
			if (evictable) {
				assert(curr._GetResident());
				m_evictables.erase(currEvictableIter);
			}
			else {
				if (curr._GetResident() == false) {
					lowLevelTargets.push_back(curr._GetResourcePtr());
					highLevelTargets.push_back(curr);
				}
			}
		}

		try {
			m_graphicsApi->MakeResident(lowLevelTargets);
		}
		catch (OutOfMemoryException&) {
			std::vector<gxapi::IResource*> toEvict;
			toEvict.reserve(m_evictables.size());
			for (auto& curr : m_evictables) {
				// NOTE I'm not sure why 'curr' is a const reference. I had to add a const cast.
				toEvict.push_back(const_cast<gxapi::IResource*>(curr._GetResourcePtr()));
			}
			m_graphicsApi->Evict(toEvict);
			m_evictables.clear();

			m_graphicsApi->MakeResident(lowLevelTargets);
		}
	}

	assert(lowLevelTargets.size() == highLevelTargets.size());
	for (size_t i = 0; i < highLevelTargets.size(); i++) {
		auto currLowlevel = lowLevelTargets[i];
		auto& currHighLevel = highLevelTargets[i];
		assert(currHighLevel._GetResourcePtr() == currLowlevel);

		currHighLevel._SetResident(true);
	}
}


template<typename IterT>
void MemoryManager::UnlockResident(IterT begin, IterT end) {
	static_assert(std::is_same<typename IterT::value_type, MemoryObject>::value);

	std::lock_guard<std::mutex> lock(m_evictablesMtx);
	m_evictables.insert(begin, end);
}

} // namespace gxeng
} // namespace inl
