#include "ScratchSpacePool.hpp"
#include <cassert>


namespace inl {
namespace gxeng {

ScratchSpacePool::ScratchSpacePool(gxapi::IGraphicsApi* gxApi, gxapi::eDescriptorHeapType type) 
	: m_gxApi(gxApi), m_type(type)
{}


auto ScratchSpacePool::RequestScratchSpace() -> UniquePtr {
	size_t index;
	try {
		index = m_allocator.Allocate();
	}
	catch (std::bad_alloc&) {
		size_t currentSize = m_pool.size();
		size_t newSize = std::max(currentSize + 1, size_t(currentSize * 1.25));
		m_pool.resize(newSize);
		m_allocator.Resize(newSize);

		index = m_allocator.Allocate();
	}

	if (m_pool[index] != nullptr) {
		return UniquePtr{ m_pool[index].get(), Deleter{this} };
	}
	else {
		std::unique_ptr<ScratchSpace> ptr(new ScratchSpace{ m_gxApi, m_type, 1000 });
		m_addressToIndex[ptr.get()] = index;
		m_pool[index] = std::move(ptr);
		return UniquePtr{ m_pool[index].get(), Deleter{this} };
	}
}


void ScratchSpacePool::RecycleScratchSpace(ScratchSpace* scratchSpace) {
	assert(m_addressToIndex.count(scratchSpace) > 0);
	size_t index = m_addressToIndex[scratchSpace];
	m_allocator.Deallocate(index);
	scratchSpace->Reset();
}



} // namespace gxeng
} // namespace inl