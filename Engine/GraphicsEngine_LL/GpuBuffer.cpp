
#include "GpuBuffer.hpp"

#include "../GraphicsApi_LL/ICommandList.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

#include "MemoryManager.hpp"
#include "ResourceHeap.hpp"

#include <utility>

namespace inl {
namespace gxeng {

using namespace gxapi;

//==================================
//Generic Resource


GenericResource::GenericResource(GenericResource&& other) :
	m_resource(other.m_resource),
	m_deleter(std::move(other.m_deleter)),
	m_resourceView(std::move(other.m_resourceView)),
	m_resident(other.m_resident)
{
	other.m_resource = nullptr;
	other.m_deleter = nullptr;
}


GenericResource& GenericResource::operator=(GenericResource&& other) noexcept {
	if (this == &other) {
		return *this;
	}

	m_resource = other.m_resource;
	m_deleter = std::move(other.m_deleter);
	m_resourceView = std::move(m_resourceView);
	m_resident = other.m_resident;

	other.m_resource = nullptr;
	other.m_deleter = nullptr;

	return *this;
}


GenericResource::~GenericResource() {
	if (m_deleter) {
		m_deleter(this);
	}
}


void* GenericResource::GetVirtualAddress() const {
	return m_resource->GetGPUAddress();
}


gxapi::ResourceDesc GenericResource::GetDescription() const {
	return m_resource->GetDesc();
}


gxapi::DescriptorHandle GenericResource::GetViewHandle() {
	return m_resourceView.Get();
}


GenericResource::GenericResource(DescriptorReference&& resourceView) :
	m_resourceView(std::move(resourceView))
{}


void GenericResource::RecordState(unsigned subresource, gxapi::eResourceState newState) {
	assert(subresource < m_subresourceStates.size());
	m_subresourceStates[subresource] = newState;
}

void GenericResource::RecordState(gxapi::eResourceState newState) {
	for (auto& state : m_subresourceStates) {
		state = newState;
	}
}

gxapi::eResourceState GenericResource::ReadState(unsigned subresource) const {
	assert(subresource < m_subresourceStates.size());
	return m_subresourceStates[subresource];
}

void GenericResource::InitResourceStates(unsigned numSubresources, gxapi::eResourceState initialState) {
	m_subresourceStates.resize(numSubresources, initialState);
}

//==================================


uint64_t LinearBuffer::GetSize() const {
	return m_resource->GetDesc().bufferDesc.sizeInBytes;
}


uint64_t GenericTextureBase::GetWidth() const {
	return m_resource->GetDesc().textureDesc.width;
}


gxapi::eFormat GenericTextureBase::GetFormat() const {
	return m_resource->GetDesc().textureDesc.format;
}


uint16_t Texture1D::GetArrayCount() const {
	return m_resource->GetDesc().textureDesc.depthOrArraySize;
}


uint64_t Texture2D::GetHeight() const {
	return m_resource->GetDesc().textureDesc.height;
}


uint16_t Texture2D::GetArrayCount() const {
	return m_resource->GetDesc().textureDesc.depthOrArraySize;
}


uint64_t Texture3D::GetHeight() const {
	return m_resource->GetDesc().textureDesc.height;
}


uint16_t Texture3D::GetDepth() const {
	return m_resource->GetDesc().textureDesc.depthOrArraySize;
}


uint64_t TextureCube::GetHeight() const {
	return m_resource->GetDesc().textureDesc.height;
}



} // namespace gxeng
} // namespace inl
