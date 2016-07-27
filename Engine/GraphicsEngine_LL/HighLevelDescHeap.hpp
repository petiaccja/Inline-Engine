#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"
#include "../BaseLibrary/Memory/RingAllocationEngine.hpp"
#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"

namespace inl {
namespace gxeng {

class HighLevelDescHeap;

class DescriptorReference {
public:
	DescriptorReference(const DescriptorReference&) = delete;
	DescriptorReference& operator=(const DescriptorReference&) = delete;

	DescriptorReference(DescriptorReference&&);
	DescriptorReference& operator=(DescriptorReference&&);


	/// <summary> Get the underlying descriptor. </summary>
	/// <exception cref="inl::gxapi::InvalidStateException">
	/// If called after the heap was resized (resizing a heap invalidates all of its contents).
	/// Also if this reference was "moved" as in move semantics.
	/// </exception>
	/// <returns> Represented descriptor. </returns>
	gxapi::DescriptorHandle Get();

protected:
	DescriptorReference(HighLevelDescHeap* home);

	bool IsInvalid() const;

protected:
	HighLevelDescHeap* m_homeHeap;
	size_t m_heapVersion;
	size_t m_position;
};


class TextureSpaceRef : public DescriptorReference {
public:
	friend class HighLevelDescHeap;

	TextureSpaceRef(const TextureSpaceRef&);
	TextureSpaceRef& operator=(TextureSpaceRef);

	TextureSpaceRef(TextureSpaceRef&&);

	~TextureSpaceRef() noexcept;
protected:
	TextureSpaceRef(exc::SlabAllocatorEngine* homeAllocator, HighLevelDescHeap* home, size_t heapVersion);
protected:
	exc::SlabAllocatorEngine* m_homeAllocator;
};


class ScratchSpaceRef : public DescriptorReference {
public:
	friend class HighLevelDescHeap;

	ScratchSpaceRef(ScratchSpaceRef&&);
	ScratchSpaceRef& operator=(ScratchSpaceRef&&);

	~ScratchSpaceRef() noexcept;
protected:
	ScratchSpaceRef(exc::RingAllocationEngine* homeAllocator, HighLevelDescHeap* home, size_t heapVersion);
protected:
	exc::RingAllocationEngine* m_homeAllocator;
};


/*
Name is subject to change
*/
class HighLevelDescHeap
{
public:
	friend class DescriptorReference;
	friend class TextureSpaceRef;
	friend class ScratchSpaceRef;

	static constexpr float DEFAULT_TEXTURE_TO_SCRATH_RATIO = 2.f;

public:
	HighLevelDescHeap(gxapi::IGraphicsApi* graphicsApi, size_t totalDescCount);

	/// <summary>Resizes this heap. ALL descriptor references will become invalid.</summary>
	void ResizeRatio(size_t totalDescCount, float textureToScratchRatio = DEFAULT_TEXTURE_TO_SCRATH_RATIO);

	/// <summary>Resizes this heap. ALL descriptor references will become invalid.</summary>
	void ResizeExplicit(size_t textureSpaceSize, size_t scratchSpaceSize);

	TextureSpaceRef CreateOnTextureSpace();
	ScratchSpaceRef CreateOnScratchSpace(size_t count);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	std::unique_ptr<gxapi::IDescriptorHeap> m_heap;
	size_t m_version; // Each resize increases version number. Zero is reserved for invalid state.

	exc::SlabAllocatorEngine m_textureSpaceAllocator; //TODO rename
	exc::RingAllocationEngine m_scratchSpaceAllocator;

protected:
	static constexpr size_t INVALID_VERSION = 0;

protected:
	void IncrementVersion();
};


} // namespace inl
} // namespace gxeng
