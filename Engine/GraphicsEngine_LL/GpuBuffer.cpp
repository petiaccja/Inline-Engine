
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
{}


GenericResource::GenericResource(DescriptorReference&& resourceView, gxapi::IResource* resource, const Deleter& deleter) :
	m_resourceView(std::move(resourceView)),
	m_deleter(deleter),
	m_resource(resource, m_deleter)
{}


GenericResource::GenericResource(GenericResource&& other) :
	m_resourceView(std::move(other.m_resourceView)),
	m_deleter(std::move(other.m_deleter)),
	m_resource(other.m_resource.release(), m_deleter),
	m_resident(other.m_resident)
{
}


GenericResource& GenericResource::operator=(GenericResource&& other) {
	if (this == &other) {
		return *this;
	}

	m_resourceView = std::move(m_resourceView);
	m_deleter = std::move(other.m_deleter);
	m_resource = decltype(m_resource){other.m_resource.release(), m_deleter};
	m_resident = other.m_resident;

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
