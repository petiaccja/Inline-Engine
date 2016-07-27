#include "HighLevelDescHeap.hpp"

#include <cassert>

namespace inl {
namespace gxeng {


DescriptorReference::DescriptorReference(DescriptorReference&& other) {
	m_position = other.m_position;
	m_heapVersion = other.m_heapVersion;

	other.m_position = 0;
	other.m_heapVersion = HighLevelDescHeap::INVALID_VERSION;
}


DescriptorReference& DescriptorReference::operator=(DescriptorReference&& other) {
	if (this == &other) {
		return *this;
	}

	m_position = other.m_position;
	m_heapVersion = other.m_heapVersion;

	other.m_position = 0;
	other.m_heapVersion = HighLevelDescHeap::INVALID_VERSION;

	return *this;
}


gxapi::DescriptorHandle DescriptorReference::Get() {
	if (IsInvalid()) {
		throw gxapi::InvalidState("Descriptor being dereferenced is INVALID!");
	}

	return m_homeHeap->m_heap->At(m_position);
}


DescriptorReference::DescriptorReference(HighLevelDescHeap* home) :
	m_homeHeap(home),
	m_heapVersion(HighLevelDescHeap::INVALID_VERSION)
{}


bool DescriptorReference::IsInvalid() const {
	return m_heapVersion == HighLevelDescHeap::INVALID_VERSION || m_heapVersion < m_homeHeap->m_version;
}


//=========================================================


TextureSpaceRef::TextureSpaceRef(const TextureSpaceRef& other) :
	TextureSpaceRef(other.m_homeAllocator, other.m_homeHeap, other.m_homeHeap->m_version)
{}


TextureSpaceRef& TextureSpaceRef::operator=(TextureSpaceRef other) {
	std::swap(m_heapVersion, other.m_heapVersion);
	std::swap(m_homeAllocator, other.m_homeAllocator);
	std::swap(m_homeHeap, other.m_homeHeap);
	std::swap(m_position, other.m_position);

	return *this;
}


TextureSpaceRef::TextureSpaceRef(TextureSpaceRef&& other) :
	DescriptorReference(std::move(other))
{
	m_homeAllocator = other.m_homeAllocator;
}


TextureSpaceRef::~TextureSpaceRef() {
	if (IsInvalid()) {
		return;
	}
	m_homeAllocator->Deallocate(m_position);
}


TextureSpaceRef::TextureSpaceRef(exc::SlabAllocatorEngine* homeAllocator, HighLevelDescHeap* home, size_t heapVersion) :
	DescriptorReference(home),
	m_homeAllocator(homeAllocator)
{
	m_position = m_homeAllocator->Allocate();
	m_heapVersion = heapVersion;
}


//=========================================================


ScratchSpaceRef::ScratchSpaceRef(ScratchSpaceRef&& other) :
	DescriptorReference(std::move(other))
{
	m_homeAllocator = other.m_homeAllocator;
}


ScratchSpaceRef& ScratchSpaceRef::operator=(ScratchSpaceRef&& other) {
	if (this == &other) {
		return *this;
	}

	DescriptorReference::operator=(std::move(other));
	m_homeAllocator = other.m_homeAllocator;

	return *this;
}


ScratchSpaceRef::~ScratchSpaceRef() {
	if (IsInvalid()) {
		return;
	}
	m_homeAllocator->Deallocate(m_position);
}


ScratchSpaceRef::ScratchSpaceRef(exc::RingAllocationEngine* homeAllocator, HighLevelDescHeap* home, size_t heapVersion) :
	DescriptorReference(home),
	m_homeAllocator(homeAllocator)
{
	m_position = m_homeAllocator->Allocate();
	m_heapVersion = heapVersion;
}


//=========================================================


HighLevelDescHeap::HighLevelDescHeap(gxapi::IGraphicsApi * graphicsApi, size_t totalDescCount) :
	m_graphicsApi(graphicsApi),
	m_version(1),
	m_textureSpaceAllocator(totalDescCount * (DEFAULT_TEXTURE_TO_SCRATH_RATIO/(1+DEFAULT_TEXTURE_TO_SCRATH_RATIO))),
	m_scratchSpaceAllocator(totalDescCount-m_textureSpaceAllocator.Size())
{
	assert(m_textureSpaceAllocator.Size()+m_scratchSpaceAllocator.GetPoolSize() == totalDescCount);

	gxapi::DescriptorHeapDesc desc;
	desc.isShaderVisible = true;
	desc.numDescriptors = totalDescCount;
	desc.type = gxapi::eDesriptorHeapType::CBV_SRV_UAV;

	m_heap.reset(graphicsApi->CreateDescriptorHeap(desc));
}


void HighLevelDescHeap::ResizeRatio(size_t totalDescCount, float textureToScratchRatio) {
	size_t textureSpaceSize = totalDescCount * (textureToScratchRatio/(1+textureToScratchRatio));
	ResizeExplicit(textureSpaceSize, totalDescCount-textureSpaceSize);
}


void HighLevelDescHeap::ResizeExplicit(size_t textureSpaceSize, size_t scratchSpaceSize) {
	m_textureSpaceAllocator.Resize(textureSpaceSize);
	m_scratchSpaceAllocator.Resize(scratchSpaceSize);

	// Lets free all slots
	m_textureSpaceAllocator.Reset();
	m_scratchSpaceAllocator.Reset();

	gxapi::DescriptorHeapDesc desc;
	desc.isShaderVisible = true;
	desc.numDescriptors = textureSpaceSize+scratchSpaceSize;
	desc.type = gxapi::eDesriptorHeapType::CBV_SRV_UAV;

	m_heap.reset(m_graphicsApi->CreateDescriptorHeap(desc));
	IncrementVersion();
}


TextureSpaceRef HighLevelDescHeap::CreateOnTextureSpace() {
	return TextureSpaceRef(&m_textureSpaceAllocator, this, m_version);
}


ScratchSpaceRef HighLevelDescHeap::CreateOnScratchSpace(size_t count) {
	return ScratchSpaceRef(&m_scratchSpaceAllocator, this, m_version);
}


void HighLevelDescHeap::IncrementVersion() {
	m_version += 1;

	// version number overflowed
	if (m_version == INVALID_VERSION) {
		throw gxapi::Exception("Operation on descriptor heap can not be done due to an internal error (version number overflowed)");
	}
}



} // namespace inl
} // namespace gxeng
