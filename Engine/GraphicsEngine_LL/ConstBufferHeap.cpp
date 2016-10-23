#include "ConstBufferHeap.hpp"

#include <cassert>

namespace inl {
namespace gxeng {


ConstantBufferHeap::ConstantBufferHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{
	m_pages.PushFront(CreatePage());
}


VolatileConstBuffer ConstantBufferHeap::CreateVolatileBuffer(void* data, uint32_t dataSize) {
	size_t targetSize = AlignUp(dataSize, ALIGNEMENT);

	std::lock_guard<std::mutex> lock(m_mutex);

	if (HasBecomeAvailable(m_pages.Front())) {
		m_pages.Front().m_consumedSize = 0; // mark it empty
	}

	ConstBufferPage* targetPage = nullptr;

	if (m_pages.Front().m_consumedSize + targetSize > m_pages.Front().m_pageSize) {
		if (targetSize > PAGE_SIZE) {
			if (m_largePages.Count() == 0) {
				m_largePages.PushFront(std::move(CreateLargePage(targetSize)));
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
					if (currPage.m_consumedSize + targetSize <= currPage.m_pageSize) {
						break; // current front will be selected as the target page, see below
					}
				}

				bool noSuitable = roundEnd == m_largePages.Begin();
				if (noSuitable) {
					m_largePages.PushFront(std::move(CreateLargePage(targetSize)));
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
				m_pages.PushFront(CreatePage());
			}

			targetPage = &m_pages.Front();
		}
	}
	else {
		targetPage = &m_pages.Front();
	}

	assert(targetPage != nullptr);

	// set owner to mach latest data that is being
	// used from the page
	targetPage->m_ownerFrameID = m_currFrameID;
	size_t offset = targetPage->m_consumedSize;
	targetPage->m_consumedSize += targetSize;

	void* cpuPtr = ((uint8_t*)targetPage->m_cpuAddress) + offset;
	void* gpuPtr = ((uint8_t*)targetPage->m_gpuAddress) + offset;

	memcpy(cpuPtr, data, dataSize);

	return VolatileConstBuffer(targetPage->m_representedMemory.get(), gpuPtr, dataSize);
}


PersistentConstBuffer ConstantBufferHeap::CreatePersistentBuffer(void* data, uint32_t dataSize) {
	std::unique_ptr<gxapi::IResource> resource{
		m_graphicsApi->CreateCommittedResource(
			gxapi::HeapProperties{gxapi::eHeapType::UPLOAD},
			gxapi::eHeapFlags::NONE,
			gxapi::ResourceDesc::Buffer(dataSize),
			gxapi::eResourceState::GENERIC_READ
		)
	};

	gxapi::MemoryRange noReadRange{0, 0};
	void* dst = resource->Map(0, &noReadRange);
	memcpy(dst, data, dataSize);
	resource->Unmap(0, nullptr);

	void* gpuPtr = resource->GetGPUAddress();

	return PersistentConstBuffer(resource.release(), gpuPtr, dataSize);
}


void ConstantBufferHeap::OnFrameBeginDevice(uint64_t frameId)
{}


void ConstantBufferHeap::OnFrameBeginHost(uint64_t frameId)
{}


void ConstantBufferHeap::OnFrameCompleteDevice(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mutex);

	m_lastFinishedFrameID++;

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


void ConstantBufferHeap::OnFrameCompleteHost(uint64_t frameId) {
	m_currFrameID++;
}


size_t ConstantBufferHeap::AlignUp(size_t value, size_t alignement) {
	// alignement should be power of two
	assert(((alignement-1) & alignement) == 0);
	return (value + (alignement-1)) & ~(alignement-1);
}


ConstantBufferHeap::ConstBufferPage ConstantBufferHeap::CreatePage() {
	return CreateLargePage(PAGE_SIZE);
}


ConstantBufferHeap::ConstBufferPage ConstantBufferHeap::CreateLargePage(size_t fittingSize) {
	const size_t resourceSize = AlignUp(fittingSize, ALIGNEMENT);
	std::unique_ptr<gxapi::IResource> resource{
		m_graphicsApi->CreateCommittedResource(
			gxapi::HeapProperties{gxapi::eHeapType::UPLOAD},
			gxapi::eHeapFlags::NONE,
			gxapi::ResourceDesc::Buffer(resourceSize),
			gxapi::eResourceState::GENERIC_READ
		)
	};

	gxapi::MemoryRange noReadRange{0, 0};
	void* cpuAddress = resource->Map(0, &noReadRange);
	void* gpuAddress = resource->GetGPUAddress();

	ConstBufferPage newPage{std::move(resource), cpuAddress, gpuAddress, resourceSize, m_currFrameID};
	return newPage;
}


bool ConstantBufferHeap::HasBecomeAvailable(const ConstBufferPage& page) {
	return page.m_ownerFrameID <= m_lastFinishedFrameID;
}

} // namespace gxeng
} // namespace inl
