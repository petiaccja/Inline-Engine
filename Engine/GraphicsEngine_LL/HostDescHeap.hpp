#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"
#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"

#include <vector>
#include <mutex>
#include <functional>
#include <cassert>

namespace inl {
namespace gxeng {

class MemoryObject;


class IHostDescHeap {
public:
	virtual size_t Allocate() = 0;
	virtual void Deallocate(size_t pos) = 0;
	virtual gxapi::DescriptorHandle At(size_t pos) = 0;
};

/// <summary>
/// This class was made for high level engine components
/// that need a way of handling resource descriptors.
/// <para />
/// This class is thread safe.
/// <para />
/// The heap automatically grows if current size is not
/// sufficient for a new allocation.
/// When the heap grows all the pointers to previous allocation remain valid.
/// This object can only represent non-shader visible heaps
/// due to the fact, that growing is implemented by creating
/// a new descriptor heap (but only one shader visible heap can be bound at a time).
/// </summary>
template <gxapi::eDescriptorHeapType HeapType>
class HostDescHeap : public IHostDescHeap {
	static constexpr size_t chunkDim = 64;
	struct ChunkListItem {
		std::unique_ptr<gxapi::IDescriptorHeap> heaps[chunkDim];
		std::unique_ptr<ChunkListItem> next;
	};

public:
	HostDescHeap(gxapi::IGraphicsApi* graphicsApi, size_t heapSize);

	size_t Allocate() override;
	void Deallocate(size_t pos) override;
	gxapi::DescriptorHandle At(size_t pos) override;
private:
	void Grow();

protected:
	gxapi::IGraphicsApi* const m_graphicsApi;
private:
	std::mutex m_listMutex;
	std::unique_ptr<ChunkListItem> m_first;
	size_t m_descriptorCount;

	std::mutex m_allocMutex;
	exc::SlabAllocatorEngine m_allocEngine;

	const size_t heapDim;
};


template <gxapi::eDescriptorHeapType HeapType>
HostDescHeap<HeapType>::HostDescHeap(gxapi::IGraphicsApi* graphicsApi, size_t heapSize)
	: m_graphicsApi(graphicsApi),
	heapDim(heapSize),
	m_allocEngine(0),
	m_descriptorCount(0)
{}

template <gxapi::eDescriptorHeapType HeapType>
size_t HostDescHeap<HeapType>::Allocate() {
	try {
		std::lock_guard<std::mutex> lkg(m_allocMutex);
		return m_allocEngine.Allocate();
	}
	catch (std::bad_alloc&) {
		std::lock_guard<std::mutex> lkg(m_listMutex);
		Grow();
		return m_allocEngine.Allocate();
	}
}

template <gxapi::eDescriptorHeapType HeapType>
void HostDescHeap<HeapType>::Deallocate(size_t pos) {
	std::lock_guard<std::mutex> lkg(m_allocMutex);
	m_allocEngine.Deallocate(pos);
}

template <gxapi::eDescriptorHeapType HeapType>
gxapi::DescriptorHandle HostDescHeap<HeapType>::At(size_t pos) {
	assert(pos < m_descriptorCount);

	// structure is like a 3D texture
	const size_t chunkIdx = pos / (chunkDim*chunkDim); // z = i / (width*height)
	const size_t heapIdx = (pos - chunkIdx*heapDim*chunkDim) / heapDim; // y = (i - z*width*height) / width
	const size_t descIdx = (pos - chunkIdx*heapDim*chunkDim - heapIdx*heapDim) / 1; // x = (i - z*width*height - y*width) / 1

	ChunkListItem* chunk = m_first.get();
	for (size_t i = chunkIdx; i != 0; --i) {
		chunk = chunk->next.get();
	}

	gxapi::IDescriptorHeap* heap = chunk->heaps[heapIdx].get();
	gxapi::DescriptorHandle desc = heap->At(descIdx);

	return desc;
}

template <gxapi::eDescriptorHeapType HeapType>
void HostDescHeap<HeapType>::Grow() {
	ChunkListItem* chunk = m_first.get();

	// if there aren't any chunks
	if (chunk == nullptr) {
		m_first = std::make_unique<ChunkListItem>();
		chunk = m_first.get();
	}

	// find last chunk
	while (chunk->next) {
		chunk = chunk->next.get();
	}

	// find last heap
	ptrdiff_t heapIdx = 0;
	while (heapIdx < chunkDim && chunk->heaps[heapIdx]) {
		++heapIdx;
	}

	// add new chunk if needed
	if (heapIdx >= chunkDim) {
		chunk->next = std::make_unique<ChunkListItem>();
		chunk = chunk->next.get();
		heapIdx = 0;
	}

	// allocate new heap
	chunk->heaps[heapIdx].reset(m_graphicsApi->CreateDescriptorHeap({ HeapType, heapDim, false }));
	m_descriptorCount += heapDim;
	m_allocEngine.Resize(m_descriptorCount);
}




class RTVHeap : public HostDescHeap<gxapi::eDescriptorHeapType::RTV> {
public:
	RTVHeap(gxapi::IGraphicsApi* graphicsApi);

	void Create(MemoryObject& resource, gxapi::RenderTargetViewDesc desc, gxapi::DescriptorHandle destination);
};


class DSVHeap : public HostDescHeap<gxapi::eDescriptorHeapType::DSV> {
public:
	DSVHeap(gxapi::IGraphicsApi* graphicsApi);

	void Create(MemoryObject& resource, gxapi::DepthStencilViewDesc desc, gxapi::DescriptorHandle destination);
};



class CbvSrvUavHeap : public HostDescHeap<gxapi::eDescriptorHeapType::CBV_SRV_UAV> {
public:
	CbvSrvUavHeap(gxapi::IGraphicsApi* graphicsApi);

	CbvSrvUavHeap(CbvSrvUavHeap&&) = default;

	void CreateCBV(gxapi::ConstantBufferViewDesc desc, gxapi::DescriptorHandle destination);
	void CreateSRV(MemoryObject& resource, gxapi::ShaderResourceViewDesc desc, gxapi::DescriptorHandle destination);
};


} // namespace gxeng
} // namespace inl

