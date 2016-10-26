#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"
#include "../BaseLibrary/Memory/RingAllocationEngine.hpp"
#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"

#include <vector>
#include <mutex>
#include <functional>

namespace inl {
namespace gxeng {

class HostDescHeap;
class ScratchSpace;
class GenericResource;

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


class ScratchSpaceRef {
public:
	friend class ScratchSpace;

	ScratchSpaceRef(ScratchSpaceRef&&);
	ScratchSpaceRef& operator=(ScratchSpaceRef&&);
	~ScratchSpaceRef() noexcept;

	/// <summary> Get an underlying descriptor. </summary>
	/// <exception cref="inl::gxapi::InvalidStateException">
	/// If this reference was "moved" as in move semantics.
	/// Or if position is outside the allocation range.
	/// </exception>
	/// <returns> Represented descriptor. </returns>
	gxapi::DescriptorHandle Get(size_t position);

	bool IsValid() const;

protected:
	ScratchSpaceRef(ScratchSpace* home, size_t pos, size_t allocSize);

	void Invalidate();

protected:
	ScratchSpace* m_home;
	size_t m_pos;
	size_t m_allocationSize;

	static constexpr auto INVALID_POS = std::numeric_limits<size_t>::max();
};

/// <summary>
/// This class provides an abstraction ovear a shader visible heap
/// that was meant to be used for draw commands.
/// <para />
/// Please note that this class is not thread safe.
/// <para />
/// Each CPU thread that generates command lists should have
/// exclusive ownership over at least one instance of this class.
/// </summary>
class ScratchSpace {
	friend class HostDescHeap;
	friend class ScratchSpaceRef;
public:
	ScratchSpace(gxapi::IGraphicsApi* graphicsApi, size_t size);

	ScratchSpaceRef Allocate(size_t size);
	DescriptorReference AllocateSingle();
protected:
	std::unique_ptr<gxapi::IDescriptorHeap> m_heap;
	exc::RingAllocationEngine m_allocator;
};


/// <summary>
/// This class was made for high level engine components
/// that need a way of handling resource descriptors.
/// <para />
/// This class is thread safe.
/// </summary>
/// (Name is subject to change)
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

	DescriptorReference Create(GenericResource& resource, gxapi::RenderTargetViewDesc desc);
};


class DSVHeap : protected HostDescHeap {
public:
	DSVHeap(gxapi::IGraphicsApi* graphicsApi);

	DescriptorReference Create(GenericResource& resource, gxapi::DepthStencilViewDesc desc);
};


class PersistentResViewHeap : protected HostDescHeap {
public:
	PersistentResViewHeap(gxapi::IGraphicsApi* graphicsApi);

	DescriptorReference CreateCBV(gxapi::ConstantBufferViewDesc desc);
	DescriptorReference CreateSRV(GenericResource& resource, gxapi::ShaderResourceViewDesc desc);
};

} // namespace gxeng
} // namespace inl
 
