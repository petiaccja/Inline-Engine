#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"

#include <limits>
#include <cstdint>

namespace inl::gxeng {

class StackDescHeap;

class DescriptorArrayRef {
public:
	friend class StackDescHeap;

	DescriptorArrayRef();

	/// <summary> Get an underlying descriptor. </summary>
	/// <exception cref="inl::gxapi::InvalidStateException">
	/// If this reference was "moved" as in move semantics.
	/// Or if position is outside the allocation range.
	/// </exception>
	/// <returns> Represented descriptor. </returns>
	gxapi::DescriptorHandle Get(uint32_t position);

	uint32_t Count() const;

	bool IsValid() const;

protected:
	DescriptorArrayRef(StackDescHeap* home, uint32_t pos, uint32_t allocSize);

protected:
	StackDescHeap* m_home;
	uint32_t m_pos;
	uint32_t m_allocationSize;

	static constexpr auto INVALID_POS = std::numeric_limits<uint32_t>::max();
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
class StackDescHeap {
	friend class DescriptorArrayRef;
public:
	StackDescHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType type, uint32_t size);

	DescriptorArrayRef Allocate(uint32_t size);

	/// <summary>
	/// Frees all allocations. Next allocation will be placed at the begginning of the heap.
	/// </summary>
	void Reset();

	gxapi::IDescriptorHeap* GetHeap() const { return m_heap.get(); }
protected:
	std::unique_ptr<gxapi::IDescriptorHeap> m_heap;
	uint32_t m_size;
	uint32_t m_next;
};


} // namespace gxeng
