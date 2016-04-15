
#include "ConstBufferManager.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

#include <cassert>

namespace inl {
namespace gxeng {

using namespace inl::gxapi;

ConstBufferManager::ConstBufferManager(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi{graphicsApi}
{}


// this function might be called from infinite threads at the same time
DisposableConstBuffer ConstBufferManager::GetDisposableBuffer(size_t size) {
	size = AlignUp(size, ALIGNEMENT);

	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_pages.Front().m_age > m_backBufferCount) {
		m_pages.Front().m_consumedSize = 0; // mark it free
	}

	if (m_pages.Front().m_consumedSize + size > m_pages.Front().m_pageSize) {
		if (size > PAGE_SIZE) {
			throw inl::gxapi::OutOfMemory("Requested buffer size is bigger than page size. Buffer can not be created. Max size (page size) is: " + std::to_string(PAGE_SIZE));
		}

		m_pages.PushFront(std::move(CreatePage()));
	}

	auto& front = m_pages.Front();
	// reset age to mach latest data that is being
	// used from the page
	front.m_age = 0;
	size_t offset = front.m_consumedSize;
	front.m_consumedSize += size;
	DisposableConstBuffer result{&front, offset, size};

	m_pages.RotateFront();

	return result;
}

void ConstBufferManager::FrameCompleted() {
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto& curr : m_pages) {
		curr.m_age += 1;
	}
}

size_t ConstBufferManager::AlignUp(size_t value, size_t alignement) {
	// alignement should be power of two
	assert(((alignement-1) & alignement) == 0);
	return (value + (alignement-1)) & ~(alignement-1);
}

ConstBufferPage ConstBufferManager::CreatePage() {
	ResourceDesc bufferDesc;
	bufferDesc.type = eResourceType::BUFFER;
	bufferDesc.bufferDesc.sizeInBytes = PAGE_SIZE;
	std::unique_ptr<gxapi::IResource> resource;
	resource.reset(m_graphicsApi->CreateCommittedResource(HeapProperties{eHeapType::UPLOAD}, eHeapFlags::NONE, bufferDesc, eResourceState::GENERIC_READ));

	gxapi::MemoryRange noReadRange{0, 0};
	void* cpuAddress = resource->Map(0, &noReadRange);
	void* gpuAddress = resource->GetGPUAddress();

	ConstBufferPage newPage{std::move(resource), cpuAddress, gpuAddress, PAGE_SIZE};
	return newPage;
}


} // namespace gxeng
} // namespace inl
