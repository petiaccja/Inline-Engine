#pragma once

#include "HighLevelDescHeap.hpp"
#include "GpuBuffer.hpp"
#include "ResourceHeap.hpp"
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


class MemoryManager {
public:
	MemoryManager(gxapi::IGraphicsApi* graphicsApi, HighLevelDescHeap* heap);

	/// <summary>
	/// Makes given resources resident.
	/// </summary>
	/// <exception cref="inl::gxapi::OutOfMemory">
	/// If there is not enough free memory in the resource's appropriate
	/// memory pool for the resource to fit in.
	/// </exception>
	void LockResident(const std::vector<GenericResource*>& resources);
	template<typename IterT>
	void LockResident(IterT begin, IterT end);

	/// <summary>
	/// WRITE A DESCRIPTION
	/// </summary>
	void UnlockResident(const std::vector<GenericResource*>& resources);
	template<typename IterT>
	void UnlockResident(IterT begin, IterT end);

	ConstBuffer CreateConstBuffer(void* data, size_t size);

	VertexBuffer* CreateVertexBuffer(eResourceHeapType heap, size_t size);
	IndexBuffer* CreateIndexBuffer(eResourceHeapType heap, size_t size);
	Texture1D* CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, uint16_t arraySize = 1);
	Texture2D* CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t arraySize = 1);
	Texture3D* CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format);
	TextureCube* CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	HighLevelDescHeap* m_descHeap;
	impl::CriticalBufferHeap m_criticalHeap;

	ConstantBufferHeap m_constBufferHeap;

	std::mutex m_evictablesMtx;
	std::unordered_set<GenericResource*> m_evictables;

protected:
	impl::InitialResourceParameters AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc& desc);
};


template<typename IterT>
inline void MemoryManager::LockResident(IterT begin, IterT end) {

	std::vector<gxapi::IResource*> lowLevelTargets;
	std::vector<GenericResource*> highLevelTargets;

	//lowLevelTargets.reserve(resources.size());
	//highLevelTargets.reserve(resources.size());
	IterT currIter = begin;
	while (currIter != end) {
		auto curr = *(currIter++);
		auto currEvictableIter = m_evictables.find(curr);
		bool evictable = currEvictableIter != m_evictables.end();
		if (evictable) {
			assert(curr->_GetResident());
			m_evictables.erase(currEvictableIter);
		}
		else {
			if (curr->_GetResident() == false) {
				lowLevelTargets.push_back(curr->_GetResourcePtr());
				highLevelTargets.push_back(curr);
			}
		}
	}

	try {
		m_graphicsApi->MakeResident(lowLevelTargets);
	}
	catch (gxapi::OutOfMemory&) {
		std::vector<gxapi::IResource*> toEvict;
		toEvict.reserve(m_evictables.size());
		for (auto curr : m_evictables) {
			toEvict.push_back(curr->_GetResourcePtr());
		}
		m_graphicsApi->Evict(toEvict);
		m_evictables.clear();

		m_graphicsApi->MakeResident(lowLevelTargets);
	}

	assert(lowLevelTargets.size() == highLevelTargets.size());
	for (size_t i = 0; i < highLevelTargets.size(); i++) {
		auto currLowlevel = lowLevelTargets[i];
		auto currHighLevel = highLevelTargets[i];
		assert(currHighLevel->_GetResourcePtr() == currLowlevel);

		currHighLevel->_SetResident(true);
	}
}


template<typename IterT>
inline void MemoryManager::UnlockResident(IterT begin, IterT end) {
	static_assert(std::is_same<typename IterT::value_type, GenericResource*>::value, "Iterator type should point to \"GenericResource*\"");

	std::lock_guard<std::mutex> lock(m_evictablesMtx);
	for (auto curr = begin; curr != end; ++curr) {
		m_evictables.insert(*curr);
	}
}

} // namespace gxeng
} // namespace inl
