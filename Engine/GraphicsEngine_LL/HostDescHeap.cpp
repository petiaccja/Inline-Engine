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


// =======================================================


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
		return 256;
	case gxapi::eDescriptorHeapType::SAMPLER:
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


DescriptorReference RTVHeap::Create(MemoryObject & resource, gxapi::RenderTargetViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateRenderTargetView(resource._GetResourcePtr(), desc, descRef.Get());

	return descRef;
}


DSVHeap::DSVHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::DSV)
{}


DescriptorReference DSVHeap::Create(MemoryObject & resource, gxapi::DepthStencilViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateDepthStencilView(resource._GetResourcePtr(), desc, descRef.Get());

	return descRef;
}


CbvSrvUavHeap::CbvSrvUavHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, gxapi::eDescriptorHeapType::CBV_SRV_UAV)
{}


DescriptorReference CbvSrvUavHeap::CreateCBV(gxapi::ConstantBufferViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateConstantBufferView(desc, descRef.Get());

	return descRef;
}


DescriptorReference CbvSrvUavHeap::CreateSRV(MemoryObject& resource, gxapi::ShaderResourceViewDesc desc) {
	auto descRef = Allocate();

	m_graphicsApi->CreateShaderResourceView(resource._GetResourcePtr(), desc, descRef.Get());

	return descRef;
}



} // namespace inl
} // namespace gxeng
