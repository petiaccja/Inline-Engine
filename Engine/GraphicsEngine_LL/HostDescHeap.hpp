#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"
#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"

#include <vector>
#include <mutex>
#include <functional>

namespace inl {
namespace gxeng {

class HostDescHeap;
class MemoryObject;


class DescriptorReference {
public:
	DescriptorReference(const gxapi::DescriptorHandle& handle, const std::function<void(void)>& deleter);

	DescriptorReference(const DescriptorReference&) = delete;
	DescriptorReference& operator=(const DescriptorReference&) = delete;

	DescriptorReference(DescriptorReference&&);
	DescriptorReference& operator=(DescriptorReference&&);

	~DescriptorReference();

	/// <summary> Get the underlying descriptor. </summary>
	/// <exception cref="inl::gxapi::InvalidStateException">
	/// If this reference was "moved" as in move semantics.
	/// </exception>
	/// <returns> Represented descriptor. </returns>
	gxapi::DescriptorHandle Get();

	bool IsValid() const;

protected:
	void Invalidate();

protected:
	std::function<void(void)> m_deleter;
	gxapi::DescriptorHandle m_handle;
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
class HostDescHeap {
	static constexpr size_t chunkDim = 64;
	struct ChunkListItem {
		std::unique_ptr<gxapi::IDescriptorHeap> heaps[chunkDim];
		std::unique_ptr<ChunkListItem> next;
	};

public:
	HostDescHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType heapType, size_t heapSize);

	size_t Allocate();
	void Deallocate(size_t pos);
	gxapi::DescriptorHandle At(size_t pos);
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
	const gxapi::eDescriptorHeapType m_heapType;
};


class RTVHeap : protected HostDescHeap {
public:
	RTVHeap(gxapi::IGraphicsApi* graphicsApi);

	static void Create(MemoryObject& resource, gxapi::RenderTargetViewDesc desc);
};


class DSVHeap : protected HostDescHeap {
public:
	DSVHeap(gxapi::IGraphicsApi* graphicsApi);

	static void Create(MemoryObject& resource, gxapi::DepthStencilViewDesc desc);
};


class PersistentResViewHeap : protected HostDescHeap {
public:
	PersistentResViewHeap(gxapi::IGraphicsApi* graphicsApi);

	static void CreateCBV(gxapi::ConstantBufferViewDesc desc);
	static void CreateSRV(MemoryObject& resource, gxapi::ShaderResourceViewDesc desc);
};


} // namespace gxeng
} // namespace inl

