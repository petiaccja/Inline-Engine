
#include "ConstBufferManager.hpp"

#include "../GraphicsApi_LL/Exception.hpp"
#include "../BaseLibrary/ScalarLiterals.hpp"

#include <cassert>

namespace inl {
namespace gxeng {

using namespace inl::gxapi;

ConstBufferManager::ConstBufferManager(gxapi::IGraphicsApi* graphicsApi, uint8_t backBufferCount) :
	m_graphicsApi{graphicsApi},
	m_backBufferCount{backBufferCount}
{
	m_pages.PushFront(std::move(CreatePage()));
}


// this function might be called from infinite threads at the same time
DisposableConstBuffer ConstBufferManager::GetDisposableBuffer(size_t size) {
	size = AlignUp(size, ALIGNEMENT);

	std::lock_guard<std::mutex> lock(m_mutex);

	if (HasBecomeAvailable(m_pages.Front())) {
		m_pages.Front().m_consumedSize = 0; // mark it empty
	}

	ConstBufferPage* targetPage = nullptr;

	if (m_pages.Front().m_consumedSize + size > m_pages.Front().m_pageSize) {
		if (size > PAGE_SIZE) {
			//throw inl::gxapi::OutOfMemory("Requested buffer size is bigger than page size. Buffer can not be created. Max size (page size) is: " + std::to_string(PAGE_SIZE));
			if (m_largePages.Count() == 0) {
				m_largePages.PushFront(std::move(CreateLargePage(size)));
			}
			else {
				if (HasBecomeAvailable(m_largePages.Front())) {
					m_largePages.Front().m_consumedSize = 0;
				}

				auto roundEnd = m_largePages.End();
				for (;
					m_largePages.Begin() != roundEnd;
					m_largePages.RotateFront())
				{
					auto& currPage = m_largePages.Front();
					if (currPage.m_consumedSize + size <= currPage.m_pageSize) {
						break; // current front will be selected as the target page, see below
					}
				}

				bool noSuitable = roundEnd == m_largePages.Begin();
				if (noSuitable) {
					m_largePages.PushFront(std::move(CreateLargePage(size)));
				}
			}

			targetPage = &m_largePages.Front();
		}
		else {
			m_pages.RotateFront();

			if (HasBecomeAvailable(m_pages.Front())) {
				m_pages.Front().m_consumedSize = 0;
			}
			else {
				m_pages.PushFront(std::move(CreatePage()));
			}

			targetPage = &m_pages.Front();
		}
	}
	else {
		targetPage = &m_pages.Front();
	}

	assert(targetPage != nullptr);

	// reset age to mach latest data that is being
	// used from the page
	targetPage->m_age = 0;
	size_t offset = targetPage->m_consumedSize;
	targetPage->m_consumedSize += size;
	DisposableConstBuffer result{targetPage, offset, size};

	return result;
}


void ConstBufferManager::FrameCompleted() {
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto& curr : m_pages) {
		curr.m_age += 1;
	}

	for (auto& curr : m_largePages) {
		curr.m_age += 1;
	}

	bool foundVictim = true;
	while (m_largePages.Count() > MAX_PERMANENT_LARGE_PAGE_COUNT && foundVictim) {
		foundVictim = false;

		for (auto roundEnd = m_largePages.End();
			m_largePages.Begin() != roundEnd && !foundVictim;
			m_largePages.RotateFront())
		{
			if (HasBecomeAvailable(m_largePages.Front())) {
				foundVictim = true;
				m_largePages.PopFront();
			}
		}
	}
}


size_t ConstBufferManager::AlignUp(size_t value, size_t alignement) {
	// alignement should be power of two
	assert(((alignement-1) & alignement) == 0);
	return (value + (alignement-1)) & ~(alignement-1);
}


ConstBufferPage ConstBufferManager::CreatePage() {
	return CreateLargePage(PAGE_SIZE);
}


ConstBufferPage ConstBufferManager::CreateLargePage(size_t fittingSize) {
	ResourceDesc bufferDesc;
	bufferDesc.type = eResourceType::BUFFER;
	bufferDesc.bufferDesc.sizeInBytes = AlignUp(fittingSize, PAGE_SIZE);
	std::unique_ptr<gxapi::IResource> resource;
	resource.reset(m_graphicsApi->CreateCommittedResource(HeapProperties{eHeapType::UPLOAD},
	                                                      eHeapFlags::NONE,
	                                                      bufferDesc,
	                                                      eResourceState::GENERIC_READ));

	gxapi::MemoryRange noReadRange{0, 0};
	void* cpuAddress = resource->Map(0, &noReadRange);
	void* gpuAddress = resource->GetGPUAddress();

	ConstBufferPage newPage{std::move(resource), cpuAddress, gpuAddress, bufferDesc.bufferDesc.sizeInBytes};
	return newPage;
}

bool ConstBufferManager::HasBecomeAvailable(const ConstBufferPage& page) {
	return page.m_age > m_backBufferCount;
}


} // namespace gxeng
} // namespace inl
