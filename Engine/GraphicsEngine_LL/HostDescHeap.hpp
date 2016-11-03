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
public:
	HostDescHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType type);

	DescriptorReference Allocate();

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	const gxapi::eDescriptorHeapType m_type;

private:
	const size_t m_heapChunkSize;
	std::vector<std::unique_ptr<gxapi::IDescriptorHeap>> m_heapChunks;
	std::mutex m_mutex;
	exc::SlabAllocatorEngine m_allocator;

private:
	void DeallocateTextureSpace(size_t pos);
	gxapi::DescriptorHandle GetAtTextureSpace(size_t pos);
	void PushNewTextureSpaceChunk();
	size_t GetOptimalChunkSize(gxapi::eDescriptorHeapType type);
};


class RTVHeap : protected HostDescHeap {
public:
	RTVHeap(gxapi::IGraphicsApi* graphicsApi);

	DescriptorReference Create(MemoryObject& resource, gxapi::RenderTargetViewDesc desc);
};


class DSVHeap : protected HostDescHeap {
public:
	DSVHeap(gxapi::IGraphicsApi* graphicsApi);

	DescriptorReference Create(MemoryObject& resource, gxapi::DepthStencilViewDesc desc);
};


class PersistentResViewHeap : protected HostDescHeap {
public:
	PersistentResViewHeap(gxapi::IGraphicsApi* graphicsApi);

	DescriptorReference CreateCBV(gxapi::ConstantBufferViewDesc desc);
	DescriptorReference CreateSRV(MemoryObject& resource, gxapi::ShaderResourceViewDesc desc);
};

} // namespace gxeng
} // namespace inl
 
