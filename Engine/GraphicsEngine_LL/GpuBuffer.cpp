
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

GenericResource::GenericResource(gxapi::IResource* resource) :
	GenericResource(resource, std::default_delete<gxapi::IResource>{})
{
	InitResourceStates(eResourceState::COMMON);
}


GenericResource::GenericResource(gxapi::IResource* resource, const Deleter& deleter) :
	m_resource(resource, deleter)
{
	InitResourceStates(eResourceState::COMMON);
}


GenericResource::GenericResource(GenericResource&& other) :
	m_resource(std::move(other.m_resource)),
	m_resident(other.m_resident),
	m_subresourceStates(other.m_subresourceStates)
{
}


GenericResource& GenericResource::operator=(GenericResource&& other) {
	if (this == &other) {
		return *this;
	}

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


IndexBuffer::IndexBuffer(gxapi::IResource* resource, size_t indexCount) :
	LinearBuffer(resource),
	m_indexCount(indexCount)
{}


IndexBuffer::IndexBuffer(gxapi::IResource* resource, const Deleter& deleter, size_t indexCount) :
	LinearBuffer(resource, deleter),
	m_indexCount(indexCount)
{}


size_t IndexBuffer::GetIndexCount() const {
	return m_indexCount;
}


//==================================


ConstBuffer::ConstBuffer(DescriptorReference&& desc, gxapi::IResource* resource, void* gpuVirtualPtr) :
	LinearBuffer(resource),
	m_gpuVirtualPtr(gpuVirtualPtr),
	m_CBV(std::move(desc))
{
}


void* ConstBuffer::GetVirtualAddress() const {
	return m_gpuVirtualPtr;
}


gxapi::DescriptorHandle ConstBuffer::GetHandle() {
	return m_CBV.Get();
}


VolatileConstBuffer::VolatileConstBuffer(DescriptorReference&& desc, gxapi::IResource* resource, void * gpuVirtualPtr):
	ConstBuffer(std::move(desc), resource, gpuVirtualPtr)
{}


PersistentConstBuffer::PersistentConstBuffer(DescriptorReference&& desc, gxapi::IResource* resource, void * gpuVirtualPtr):
	ConstBuffer(std::move(desc), resource, gpuVirtualPtr)
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


BackBuffer::BackBuffer(DescriptorReference&& descRef, gxapi::RenderTargetViewDesc desc, gxapi::IResource* resource) :
	// Underlying resource deallocation is managed by the swap chain!
	Texture2D(resource, [](gxapi::IResource*){}),
	m_RTV(std::shared_ptr<Texture2D>(this, [](Texture2D*){}), std::move(descRef), desc)
{}


RenderTargetView& BackBuffer::GetView() {
	return m_RTV;
}


} // namespace gxeng
} // namespace inl
