#include "HostDescHeap.hpp"

#include "MemoryObject.hpp"

#include <cassert>
#include <array>
#include <type_traits>


namespace inl {
namespace gxeng {


DescriptorReference::DescriptorReference(const gxapi::DescriptorHandle& handle, const std::function<void(void)>& deleter) :
	m_handle(handle),
	m_deleter(deleter)
{}


DescriptorReference::DescriptorReference(DescriptorReference&& other) :
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




HostDescHeap::HostDescHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType heapType, size_t heapSize)
	: m_graphicsApi(graphicsApi),
	heapDim(heapSize),
	m_heapType(heapType),
	m_allocEngine(0),
	m_descriptorCount(0)
{}

size_t HostDescHeap::Allocate() {
	try {
		std::lock_guard<std::mutex> lkg(m_allocMutex);
		m_allocEngine.Allocate();
	}
	catch (std::bad_alloc&) {
		std::lock_guard<std::mutex> lkg(m_listMutex);
		Grow();
		m_allocEngine.Allocate();
	}
}

void HostDescHeap::Deallocate(size_t pos) {
	std::lock_guard<std::mutex> lkg(m_allocMutex);
	m_allocEngine.Deallocate(pos);
}

gxapi::DescriptorHandle HostDescHeap::At(size_t pos) {
	assert(pos < m_descriptorCount);

	// structure is like a 3D texture
	const size_t chunkIdx = pos / (chunkDim*chunkDim); // z = i / (width*height)
	const size_t heapIdx = (pos - chunkIdx*heapDim*chunkDim) / heapDim; // y = (i - z*width*height) / width
	const size_t descIdx = (pos - chunkIdx*heapDim*chunkDim - heapIdx*heapDim) / 1; // x = (i - z*width*height - y*width) / 1

	ChunkListItem* chunk = m_first.get();
	for (size_t i = chunkIdx; i != 0; --i) {
		chunk = chunk->next.get();
	}

	gxapi::IDescriptorHeap* heap = chunk->heaps[heapIdx].get();
	gxapi::DescriptorHandle desc = heap->At(descIdx);

	return desc;
}

void HostDescHeap::Grow() {
	// find last chunk
	ChunkListItem* chunk = m_first.get();
	while (chunk->next) {
		chunk = chunk->next.get();
	}

	// find last heap
	ptrdiff_t heapIdx = 0;
	while (heapIdx < chunkDim && chunk->heaps[heapIdx]) {
		++heapIdx;
	}

	// add new chunk if needed
	if (heapIdx >= chunkDim) {
		chunk->next = std::make_unique<ChunkListItem>();
		chunk = chunk->next.get();
		heapIdx = 0;
	}

	// allocate new heap
	chunk->heaps[heapIdx].reset(m_graphicsApi->CreateDescriptorHeap({m_heapType, heapDim, false}));
	m_descriptorCount += heapDim;
	m_allocEngine.Resize(m_descriptorCount);
}



RTVHeap::RTVHeap(gxapi::IGraphicsApi * graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::RTV, 32)
{}


size_t RTVHeap::Create(MemoryObject & resource, gxapi::RenderTargetViewDesc desc) {
	auto place = Allocate();

	m_graphicsApi->CreateRenderTargetView(resource._GetResourcePtr(), desc, At(place));

	return place;
}


DSVHeap::DSVHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::DSV, 16)
{}


size_t DSVHeap::Create(MemoryObject & resource, gxapi::DepthStencilViewDesc desc) {
	auto place = Allocate();

	m_graphicsApi->CreateDepthStencilView(resource._GetResourcePtr(), desc, At(place));

	return place;
}


PersistentResViewHeap::PersistentResViewHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::CBV_SRV_UAV, 256)
{}


size_t PersistentResViewHeap::CreateCBV(gxapi::ConstantBufferViewDesc desc) {
	auto place = Allocate();

	m_graphicsApi->CreateConstantBufferView(desc, At(place));

	return place;
}


size_t PersistentResViewHeap::CreateSRV(MemoryObject& resource, gxapi::ShaderResourceViewDesc desc) {
	auto place = Allocate();

	m_graphicsApi->CreateShaderResourceView(resource._GetResourcePtr(), desc, At(place));

	return place;
}



} // namespace inl
} // namespace gxeng
