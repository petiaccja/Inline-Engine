
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

GenericResource::GenericResource(DescriptorReference&& resourceView, gxapi::IResource* resource) :
	GenericResource(std::move(resourceView), resource, std::default_delete<gxapi::IResource>{})
{
	InitResourceStates(eResourceState::COMMON);
}


GenericResource::GenericResource(DescriptorReference&& resourceView, gxapi::IResource* resource, const Deleter& deleter) :
	m_resourceView(std::move(resourceView)),
	m_resource(resource, deleter)
{
	InitResourceStates(eResourceState::COMMON);
}


GenericResource::GenericResource(GenericResource&& other) :
	m_resourceView(std::move(other.m_resourceView)),
	m_resource(std::move(other.m_resource)),
	m_resident(other.m_resident),
	m_subresourceStates(other.m_subresourceStates)
{
}


GenericResource& GenericResource::operator=(GenericResource&& other) {
	if (this == &other) {
		return *this;
	}

	m_resourceView = std::move(m_resourceView);
	m_resource = std::move(other.m_resource);
	m_resident = other.m_resident;
	m_subresourceStates = std::move(other.m_subresourceStates);

	return *this;
}


void* GenericResource::GetVirtualAddress() const {
	return m_resource->GetGPUAddress();
}


gxapi::ResourceDesc GenericResource::GetDescription() const {
	return m_resource->GetDesc();
}


gxapi::DescriptorHandle GenericResource::GetHandle() {
	return m_resourceView.Get();
}


void GenericResource::_SetResident(bool value) noexcept {
	m_resident = value;
}


bool GenericResource::_GetResident() const noexcept {
	return m_resident;
}


gxapi::IResource* GenericResource::_GetResourcePtr() noexcept {
	return m_resource.get();
}


const gxapi::IResource * GenericResource::_GetResourcePtr() const noexcept {
	return m_resource.get();
}


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

void GenericResource::InitResourceStates(gxapi::eResourceState initialState) {
	gxapi::ResourceDesc desc = m_resource->GetDesc();
	unsigned numSubresources = 0;
	switch (desc.type) {
		case eResourceType::TEXTURE:
		{
			switch (desc.textureDesc.dimension) {
				case eTextueDimension::ONE:
					numSubresources = desc.textureDesc.depthOrArraySize * desc.textureDesc.mipLevels;
					break;
				case eTextueDimension::TWO:
					numSubresources = desc.textureDesc.depthOrArraySize * desc.textureDesc.mipLevels;
					break;
				case eTextueDimension::THREE:
					numSubresources = desc.textureDesc.mipLevels;
					break;
				default: assert(false);
			}
			break;
		}
		case eResourceType::BUFFER:
		{
			numSubresources = 1;
			break;
		}
		default: assert(false);
	}
	m_subresourceStates.resize(numSubresources, initialState);
}

//==================================


uint64_t LinearBuffer::GetSize() const {
	return m_resource->GetDesc().bufferDesc.sizeInBytes;
}


//==================================


ConstBuffer::ConstBuffer(DescriptorReference && resourceView, gxapi::IResource* resource, void* gpuVirtualPtr) :
	LinearBuffer(std::move(resourceView), resource),
	m_gpuVirtualPtr(gpuVirtualPtr)
{
}


void* ConstBuffer::GetVirtualAddress() const {
	return m_gpuVirtualPtr;
}


VolatileConstBuffer::VolatileConstBuffer(DescriptorReference && resourceView, gxapi::IResource* resource, void * gpuVirtualPtr):
	ConstBuffer(std::move(resourceView), resource, gpuVirtualPtr)
{}


PersistentConstBuffer::PersistentConstBuffer(DescriptorReference && resourceView, gxapi::IResource* resource, void * gpuVirtualPtr):
	ConstBuffer(std::move(resourceView), resource, gpuVirtualPtr)
{}


//==================================


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
