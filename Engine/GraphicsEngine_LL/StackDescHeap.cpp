
#include "StackDescHeap.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include <cassert>

namespace inl {
namespace gxeng {


DescriptorArrayRef::DescriptorArrayRef() :
	m_home(nullptr), m_pos(INVALID_POS), m_allocationSize(0)
{}


gxapi::DescriptorHandle DescriptorArrayRef::Get(uint32_t position) {
	if (!IsValid()) {
		throw InvalidStateException("Descriptor being dereferenced is INVALID!");
	}

	if (position >= m_allocationSize) {
		throw OutOfRangeException("Requested scratch space descriptor is out of allocation range!");
	}

	return m_home->m_heap->At(m_pos + position);
}


uint32_t DescriptorArrayRef::Count() const {
	return m_allocationSize;
}


bool DescriptorArrayRef::IsValid() const {
	return m_pos != INVALID_POS;
}


DescriptorArrayRef::DescriptorArrayRef(StackDescHeap* home, uint32_t pos, uint32_t allocSize) :
	m_home(home),
	m_pos(pos),
	m_allocationSize(allocSize)
{}


// =======================================================


StackDescHeap::StackDescHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType type, uint32_t size) :
	m_size(size),
	m_next(0)
{
	assert(type == gxapi::eDescriptorHeapType::CBV_SRV_UAV || type == gxapi::eDescriptorHeapType::SAMPLER);
	gxapi::DescriptorHeapDesc desc(type, size, true);
	m_heap.reset(graphicsApi->CreateDescriptorHeap(desc));
}


DescriptorArrayRef StackDescHeap::Allocate(uint32_t size) {
	uint32_t newNext = m_next + size;
	if (newNext > m_size) {
		throw std::bad_alloc();
	}
	uint32_t descPosition = m_next;
	m_next = newNext;
	return DescriptorArrayRef(this, descPosition, size);
}


void StackDescHeap::Reset() {
	m_next = 0;
}


} // namespace gxeng
} // namespace inl
