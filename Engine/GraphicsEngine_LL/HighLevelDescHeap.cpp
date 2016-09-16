#include "HighLevelDescHeap.hpp"

#include <cassert>
#include <array>
#include <type_traits>

namespace inl {
namespace gxeng {


DescriptorReference::DescriptorReference(const gxapi::DescriptorHandle & handle, const std::function<void(void)>& deleter) :
	m_handle(handle),
	m_deleter(deleter)
{}


DescriptorReference::DescriptorReference(DescriptorReference && other) :
	m_deleter(std::move(other.m_deleter)),
	m_handle(std::move(other.m_handle))
{
	other.Invalidate();
}


DescriptorReference& DescriptorReference::operator=(DescriptorReference&& other) {
	if (this == &other) {
		return *this;
	}

	m_deleter = std::move(other.m_deleter);
	m_handle = std::move(other.m_handle);

	other.Invalidate();

	return *this;
}


DescriptorReference::~DescriptorReference() {
	if (m_deleter) {
		m_deleter();
	}
}


gxapi::DescriptorHandle DescriptorReference::Get() {
	if (!IsValid()) {
		throw gxapi::InvalidState("Descriptor being retrieved is INVALID!");
	}

	return m_handle;
}


bool DescriptorReference::IsValid() const {
	return m_handle.cpuAddress != nullptr || m_handle.gpuAddress != nullptr;
}


void DescriptorReference::Invalidate() {
	m_handle.cpuAddress = nullptr;
	m_handle.gpuAddress = nullptr;
}



//=========================================================


#if 0
TextureSpaceRef::TextureSpaceRef(TextureSpaceRef&& other) :
	DescriptorReference(std::move(other)),
	m_home(other.m_home)
{}


TextureSpaceRef& TextureSpaceRef::operator=(TextureSpaceRef&& other) {
	if (this == &other) {
		return *this;
	}

	DescriptorReference::operator=(std::move(other));
	m_home = other.m_home;

	return *this;
}


TextureSpaceRef::~TextureSpaceRef() {
	if (IsValid()) {
		m_home->DeallocateTextureSpace(m_position);
	}
}


gxapi::DescriptorHandle TextureSpaceRef::Get() {
	if (!IsValid()) {
		throw gxapi::InvalidState("Descriptor being dereferenced is INVALID!");
	}

	return m_home->GetAtTextureSpace(m_position);
}


TextureSpaceRef::TextureSpaceRef(HighLevelDescHeap* home, size_t pos) noexcept :
	DescriptorReference(pos),
	m_home(home)
{}
#endif

//=========================================================


ScratchSpaceRef::ScratchSpaceRef(ScratchSpaceRef&& other) :
	m_home(other.m_home),
	m_pos(other.m_pos),
	m_allocationSize(other.m_allocationSize)
{
	other.Invalidate();
}


ScratchSpaceRef& ScratchSpaceRef::operator=(ScratchSpaceRef&& other) {
	if (this == &other) {
		return *this;
	}

	m_home = other.m_home;
	m_pos = other.m_pos;
	m_allocationSize = other.m_allocationSize;

	other.Invalidate();

	return *this;
}


ScratchSpaceRef::~ScratchSpaceRef() {
	if (IsValid()) {
		m_home->m_allocator.Deallocate(m_pos);
	}
}


gxapi::DescriptorHandle ScratchSpaceRef::Get(size_t position) {
	if (!IsValid()) {
		throw gxapi::InvalidState("Descriptor being dereferenced is INVALID!");
	}

	if (position >= m_allocationSize) {
		throw gxapi::OutOfRange("Requested scratch space descriptor is out of allocation range!");
	}

	return m_home->m_heap->At(m_pos + position);
}


bool ScratchSpaceRef::IsValid() const {
	return m_pos != INVALID_POS;
}


ScratchSpaceRef::ScratchSpaceRef(ScratchSpace * home, size_t pos, size_t allocSize) :
	m_home(home),
	m_pos(pos),
	m_allocationSize(allocSize)
{}


void ScratchSpaceRef::Invalidate() {
	m_pos = INVALID_POS;
}


//=========================================================


ScratchSpaceRef ScratchSpace::Allocate(size_t size) {
	return ScratchSpaceRef(this, m_allocator.Allocate(size), size);
}


ScratchSpace::ScratchSpace(gxapi::IGraphicsApi * graphicsApi, size_t size) :
	m_allocator(size)
{
	gxapi::DescriptorHeapDesc desc(gxapi::eDesriptorHeapType::CBV_SRV_UAV, size, true);
	m_heap.reset(graphicsApi->CreateDescriptorHeap(desc));
}


//=========================================================


HighLevelDescHeap::HighLevelDescHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi),
	m_textureSpaceAllocator(0)
{
	PushNewTextureSpaceChunk();
}


DescriptorReference HighLevelDescHeap::AllocateOnTextureSpace() {
	std::lock_guard<std::mutex> lock(m_textureSpaceMtx);
	size_t pos;
	try {
		pos = m_textureSpaceAllocator.Allocate();
	}
	catch (std::bad_alloc&) {
		PushNewTextureSpaceChunk();
		pos = m_textureSpaceAllocator.Allocate();
	}

	DescriptorReference result{
		GetAtTextureSpace(pos),
		[this, pos]() { m_textureSpaceAllocator.Deallocate(pos); }
	};
	return result;
}

ScratchSpace HighLevelDescHeap::CreateScratchSpace(size_t size) {
	return ScratchSpace(m_graphicsApi, size);
}


/* OLD
struct CopyRequest {
TextureSpaceRef* from;
DescriptorReference* to;

CopyRequest() = default;
CopyRequest(TextureSpaceRef* from, DescriptorReference* to) : from(from), to(to){};
};

void CopyDescriptors(const std::vector<CopyRequest>& fromToList) {
	if (fromToList.size() == 0) {
		return;
	}

	std::vector<std::vector<std::pair<size_t, size_t>>> sortableSubQueues(1);
	for (auto& currRequest : fromToList) {
		auto pCurrSubQueue = &(sortableSubQueues.back());
		bool shouldBeSeparate = false;
		for (auto& currSubRequest : *pCurrSubQueue) {
			auto& currReqFrom = currRequest.from->m_position;
			auto& currReqTo = currRequest.to->m_position;
			auto& currSubReqFrom = currSubRequest.first;
			auto& currSubReqTo = currSubRequest.second;

			assert(currReqFrom != currReqTo);

			bool readAfterWrite = currReqFrom == currSubReqTo;
			bool writeAfterRead = currReqTo == currSubReqFrom;
			if (readAfterWrite || writeAfterRead) {
				shouldBeSeparate = true;
				break;
			}
		}
		if (shouldBeSeparate) {
			sortableSubQueues.push_back({});
			pCurrSubQueue = &(sortableSubQueues.back());
		}

		pCurrSubQueue->push_back({currRequest.from->m_position, currRequest.to->m_position});
	}

	for (auto& currSubQueue : sortableSubQueues) {
		//Sorting a vector of pairs will result in a vector where the same values are neighbours
		//This helps us dividing the requests into groups so that the operations are as efficient as possible
		std::sort(currSubQueue.begin(), currSubQueue.end());

		std::vector<gxapi::DescriptorHandle> srcStarts;
		std::vector<gxapi::DescriptorHandle> dstStarts;
		std::vector<uint32_t> counts;

		//Fill out requests
		const auto invalidIter = currSubQueue.end();
		auto previous = invalidIter;
		for (auto curr = currSubQueue.begin(); curr != currSubQueue.end(); ++curr) {
			auto srcCurr = m_heap->At(curr->first);
			auto dstCurr = m_heap->At(curr->second);

			bool addNew = true;
			if (previous != invalidIter) {
				auto srcNeighborOfPrev = m_heap->At(previous->first + 1);
				auto dstNeighborOfPrev = m_heap->At(previous->second + 1);

				if (srcNeighborOfPrev == srcCurr && dstNeighborOfPrev == dstCurr) {
					addNew = false;
				}
			}

			if (addNew) {
				srcStarts.push_back(srcCurr);
				dstStarts.push_back(dstCurr);
				counts.push_back(1);
			}
			else {
				counts.back()++;
			}

			previous = curr;
		}

		assert(srcStarts.size() == dstStarts.size());
		assert(dstStarts.size() == counts.size());

		//TODO FIX
		//m_graphicsApi->CopyDescriptors(dstStarts.size(), dstStarts.data(), counts.data(), srcStarts.size(), srcStarts.data(), m_heap->GetDesc().type);
	}
}
*/


void HighLevelDescHeap::DeallocateTextureSpace(size_t pos) {
	std::lock_guard<std::mutex> lock(m_textureSpaceMtx);
	m_textureSpaceAllocator.Deallocate(pos);
}


gxapi::DescriptorHandle HighLevelDescHeap::GetAtTextureSpace(size_t pos) {
	size_t chunk = pos / TEXTURE_SPACE_CHUNK_SIZE;
	size_t id = pos % TEXTURE_SPACE_CHUNK_SIZE;

	assert(chunk < m_textureSpaceChunks.size());

	return m_textureSpaceChunks[chunk]->At(id);
}


void HighLevelDescHeap::PushNewTextureSpaceChunk() {
	gxapi::DescriptorHeapDesc desc(gxapi::eDesriptorHeapType::CBV_SRV_UAV, TEXTURE_SPACE_CHUNK_SIZE, false);
	m_textureSpaceChunks.push_back(std::unique_ptr<gxapi::IDescriptorHeap>(m_graphicsApi->CreateDescriptorHeap(desc)));
	try { // You never know
		m_textureSpaceAllocator.Resize(m_textureSpaceAllocator.Size() + TEXTURE_SPACE_CHUNK_SIZE);
	}
	catch (...) {
		m_textureSpaceChunks.pop_back();
		throw;
	}
}


} // namespace inl
} // namespace gxeng
