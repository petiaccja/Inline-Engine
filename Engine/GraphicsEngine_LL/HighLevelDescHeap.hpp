#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"
#include "../BaseLibrary/Memory/RingAllocationEngine.hpp"
#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"

#include <vector>
#include <mutex>

namespace inl {
namespace gxeng {

class HighLevelDescHeap;
class ScratchSpace;

class DescriptorReference {
public:
	friend class HighLevelDescHeap;

	DescriptorReference(const DescriptorReference&) = delete;
	DescriptorReference& operator=(const DescriptorReference&) = delete;

	DescriptorReference(DescriptorReference&&);
	DescriptorReference& operator=(DescriptorReference&&);

	bool IsValid() const;

protected:
	DescriptorReference(size_t pos) noexcept;

protected:
	size_t m_position;

	static constexpr auto INVALID_POSITION = std::numeric_limits<size_t>::max();
};


class TextureSpaceRef : public DescriptorReference {
public:
	friend class HighLevelDescHeap;

	TextureSpaceRef(TextureSpaceRef&&);
	TextureSpaceRef& operator=(TextureSpaceRef&&);
	~TextureSpaceRef() noexcept;

	/// <summary> Get the underlying descriptor. </summary>
	/// <exception cref="inl::gxapi::InvalidStateException">
	/// If this reference was "moved" as in move semantics.
	/// </exception>
	/// <returns> Represented descriptor. </returns>
	gxapi::DescriptorHandle Get();
protected:
	TextureSpaceRef(HighLevelDescHeap* home, size_t pos) noexcept;
protected:
	HighLevelDescHeap* m_home;
};


class ScratchSpaceRef : public DescriptorReference {
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
protected:
	ScratchSpaceRef(ScratchSpace* home, size_t pos, size_t allocSize);
protected:
	ScratchSpace* m_home;
	size_t m_allocationSize;
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
	friend class HighLevelDescHeap;
	friend class ScratchSpaceRef;
public:
	ScratchSpaceRef Allocate(size_t size);

protected:
	ScratchSpace(gxapi::IGraphicsApi* graphicsApi, size_t size);

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
class HighLevelDescHeap
{
public:
	friend class TextureSpaceRef;

public:
	HighLevelDescHeap(gxapi::IGraphicsApi* graphicsApi);

	TextureSpaceRef AllocateOnTextureSpace();
	ScratchSpace CreateScratchSpace(size_t size);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	static constexpr size_t TEXTURE_SPACE_CHUNK_SIZE = 256;
	std::vector<std::unique_ptr<gxapi::IDescriptorHeap>> m_textureSpaceChunks;
	std::mutex m_textureSpaceMtx; 
	exc::SlabAllocatorEngine m_textureSpaceAllocator;

protected:
	void DeallocateTextureSpace(size_t pos);
	gxapi::DescriptorHandle GetAtTextureSpace(size_t pos);
	void PushNewTextureSpaceChunk();
};


} // namespace inl
} // namespace gxeng
