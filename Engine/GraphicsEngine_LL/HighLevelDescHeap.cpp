#include "HighLevelDescHeap.hpp"

#include "GpuBuffer.hpp"

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


gxapi::DescriptorHandle ScratchSpaceRef::Get(uint32_t position) {
	if (!IsValid()) {
		throw gxapi::InvalidState("Descriptor being dereferenced is INVALID!");
	}

	if (position >= m_allocationSize) {
		throw gxapi::OutOfRange("Requested scratch space descriptor is out of allocation range!");
	}

	return m_home->m_heap->At(m_pos + position);
}


uint32_t ScratchSpaceRef::Count() const {
	return m_allocationSize;
}


bool ScratchSpaceRef::IsValid() const {
	return m_pos != INVALID_POS;
}


ScratchSpaceRef::ScratchSpaceRef(ScratchSpace * home, uint32_t pos, uint32_t allocSize) :
	m_home(home),
	m_pos(pos),
	m_allocationSize(allocSize)
{}


void ScratchSpaceRef::Invalidate() {
	m_pos = INVALID_POS;
}


//=========================================================


ScratchSpace::ScratchSpace(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType type, uint32_t size) :
	m_size(size),
	m_top(0)
{
	assert(type == gxapi::eDescriptorHeapType::CBV_SRV_UAV || type == gxapi::eDescriptorHeapType::SAMPLER);
	gxapi::DescriptorHeapDesc desc(type, size, true);
	m_heap.reset(graphicsApi->CreateDescriptorHeap(desc));
}


ScratchSpaceRef ScratchSpace::Allocate(uint32_t size) {
	uint32_t newTop = m_top + size;
	if (newTop > m_size) {
		throw std::bad_alloc();
	}
	uint32_t descPosition = m_top;
	m_top = newTop;
	return ScratchSpaceRef(this, descPosition, size);
}


void ScratchSpace::Reset() {
	m_top = 0;
}


//=========================================================


HostDescHeap::HostDescHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::eDescriptorHeapType type) :
	m_graphicsApi(graphicsApi),
	m_type(type),
	m_heapChunkSize(GetOptimalChunkSize(type)),
	m_allocator(0)
{
	PushNewTextureSpaceChunk();
}


DescriptorReference HostDescHeap::Allocate() {
	std::lock_guard<std::mutex> lock(m_mutex);
	size_t pos;
	try {
		pos = m_allocator.Allocate();
	}
	catch (std::bad_alloc&) {
		PushNewTextureSpaceChunk();
		pos = m_allocator.Allocate();
	}

	DescriptorReference result{
		GetAtTextureSpace(pos),
		[this, pos]() { std::lock_guard<std::mutex> lock(m_mutex); m_allocator.Deallocate(pos); }
	};
	return result;
}


void HostDescHeap::DeallocateTextureSpace(size_t pos) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_allocator.Deallocate(pos);
}


gxapi::DescriptorHandle HostDescHeap::GetAtTextureSpace(size_t pos) {
	size_t chunk = pos / m_heapChunkSize;
	size_t id = pos % m_heapChunkSize;

	assert(chunk < m_heapChunks.size());

	return m_heapChunks[chunk]->At(id);
}


void HostDescHeap::PushNewTextureSpaceChunk() {
	gxapi::DescriptorHeapDesc desc(m_type, m_heapChunkSize, false);
	m_heapChunks.push_back(std::unique_ptr<gxapi::IDescriptorHeap>(m_graphicsApi->CreateDescriptorHeap(desc)));
	try { // You never know
		m_allocator.Resize(m_allocator.Size() + m_heapChunkSize);
	}
	catch (...) {
		m_heapChunks.pop_back();
		throw;
	}
}


size_t HostDescHeap::GetOptimalChunkSize(gxapi::eDescriptorHeapType type) {
	switch (type) {
	case gxapi::eDescriptorHeapType::CBV_SRV_UAV:
	case gxapi::eDescriptorHeapType::SAMPLER:
		return 256;
	case gxapi::eDescriptorHeapType::RTV:
	case gxapi::eDescriptorHeapType::DSV:
		return 16;
	default:
		assert(false);
	}

	return 256;
}


RTVHeap::RTVHeap(gxapi::IGraphicsApi * graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::RTV)
{}


DescriptorReference RTVHeap::Create(GenericResource & resource, gxapi::RenderTargetViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateRenderTargetView(resource._GetResourcePtr(), desc, descRef.Get());

	return descRef;
}


DSVHeap::DSVHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::DSV)
{}


DescriptorReference DSVHeap::Create(GenericResource & resource, gxapi::DepthStencilViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateDepthStencilView(resource._GetResourcePtr(), desc, descRef.Get());

	return descRef;
}


PersistentResViewHeap::PersistentResViewHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::CBV_SRV_UAV)
{}


DescriptorReference PersistentResViewHeap::CreateCBV(gxapi::ConstantBufferViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateConstantBufferView(desc, descRef.Get());

	return descRef;
}


DescriptorReference PersistentResViewHeap::CreateSRV(GenericResource& resource, gxapi::ShaderResourceViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateShaderResourceView(resource._GetResourcePtr(), desc, descRef.Get());

	return descRef;
}



} // namespace inl
} // namespace gxeng
