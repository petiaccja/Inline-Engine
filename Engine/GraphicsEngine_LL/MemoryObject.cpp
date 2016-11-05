
#include "MemoryObject.hpp"

#include "../GraphicsApi_LL/ICommandList.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

#include "MemoryManager.hpp"
#include "CriticalBufferHeap.hpp"

#include <utility>
#include <iostream>

namespace inl {
namespace gxeng {

using namespace gxapi;

//==================================

MemoryObjDesc::MemoryObjDesc(gxapi::IResource* ptr, bool resident) :
	resource(ptr, std::default_delete<gxapi::IResource>()),
	resident(resident)
{}



MemoryObject::MemoryObject(MemoryObjDesc&& desc) :
	m_resource(std::move(desc.resource))
{
	m_resident = desc.resident;

	InitResourceStates(eResourceState::COMMON);
}


MemoryObject::MemoryObject(MemoryObject&& other) :
	m_resource(std::move(other.m_resource)),
	m_resident(other.m_resident),
	m_subresourceStates(other.m_subresourceStates)
{}


MemoryObject& MemoryObject::operator=(MemoryObject&& other) {
	if (this == &other) {
		return *this;
	}

	m_resource = std::move(other.m_resource);
	m_resident = other.m_resident;
	m_subresourceStates = std::move(other.m_subresourceStates);

	return *this;
}


void* MemoryObject::GetVirtualAddress() const {
	return m_resource->GetGPUAddress();
}


gxapi::ResourceDesc MemoryObject::GetDescription() const {
	return m_resource->GetDesc();
}


void MemoryObject::_SetResident(bool value) noexcept {
	m_resident = value;
}


bool MemoryObject::_GetResident() const noexcept {
	return m_resident;
}


gxapi::IResource* MemoryObject::_GetResourcePtr() noexcept {
	return m_resource.get();
}


const gxapi::IResource * MemoryObject::_GetResourcePtr() const noexcept {
	return m_resource.get();
}


void MemoryObject::RecordState(unsigned subresource, gxapi::eResourceState newState) {
	assert(subresource < m_subresourceStates.size());
	m_subresourceStates[subresource] = newState;
}

void MemoryObject::RecordState(gxapi::eResourceState newState) {
	for (auto& state : m_subresourceStates) {
		state = newState;
	}
}

gxapi::eResourceState MemoryObject::ReadState(unsigned subresource) const {
	assert(subresource < m_subresourceStates.size());
	return m_subresourceStates[subresource];
}

void MemoryObject::InitResourceStates(gxapi::eResourceState initialState) {
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


IndexBuffer::IndexBuffer(MemoryObjDesc&& desc, size_t indexCount) :
	LinearBuffer(std::move(desc)),
	m_indexCount(indexCount)
{}


size_t IndexBuffer::GetIndexCount() const {
	return m_indexCount;
}


//==================================


ConstBuffer::ConstBuffer(MemoryObjDesc&& desc, void* gpuVirtualPtr, uint32_t dataSize) :
	LinearBuffer(std::move(desc)),
	m_gpuVirtualPtr(gpuVirtualPtr),
	m_dataSize(dataSize)
{}


void* ConstBuffer::GetVirtualAddress() const {
	return m_gpuVirtualPtr;
}


uint64_t ConstBuffer::GetSize() const {
	return m_dataSize;
}


VolatileConstBuffer::VolatileConstBuffer(MemoryObjDesc&& desc, void * gpuVirtualPtr, uint32_t dataSize) :
	ConstBuffer(std::move(desc), gpuVirtualPtr, dataSize)
{}


PersistentConstBuffer::PersistentConstBuffer(MemoryObjDesc&& desc, void * gpuVirtualPtr, uint32_t dataSize) :
	ConstBuffer(std::move(desc), gpuVirtualPtr, dataSize)
{}


//==================================


uint64_t Texture1D::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint16_t Texture1D::GetArrayCount() const {
	return m_resource->GetDesc().textureDesc.depthOrArraySize;
}


gxapi::eFormat Texture1D::GetFormat() const {
	return GetDescription().textureDesc.format;
}


uint64_t Texture2D::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint64_t Texture2D::GetHeight() const {
	return m_resource->GetDesc().textureDesc.height;
}


uint16_t Texture2D::GetArrayCount() const {
	return m_resource->GetDesc().textureDesc.depthOrArraySize;
}


uint32_t Texture2D::GetSubresourceIndex(uint32_t arrayIndex, uint32_t mipLevel) const {
	return arrayIndex*GetDescription().textureDesc.mipLevels + mipLevel;
}


gxapi::eFormat Texture2D::GetFormat() const {
	return GetDescription().textureDesc.format;
}


uint64_t Texture3D::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint64_t Texture3D::GetHeight() const {
	return m_resource->GetDesc().textureDesc.height;
}


uint16_t Texture3D::GetDepth() const {
	return m_resource->GetDesc().textureDesc.depthOrArraySize;
}


gxapi::eFormat Texture3D::GetFormat() const {
	return GetDescription().textureDesc.format;
}


uint64_t TextureCube::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint64_t TextureCube::GetHeight() const {
	return m_resource->GetDesc().textureDesc.height;
}


gxapi::eFormat TextureCube::GetFormat() const {
	return GetDescription().textureDesc.format;
}


BackBuffer::BackBuffer(DescriptorReference&& descRef, gxapi::RenderTargetViewDesc rtvDesc, MemoryObjDesc&& objDesc) :
	Texture2D(std::move(objDesc)),
	m_RTV(std::shared_ptr<Texture2D>(this, [](Texture2D*) {}), std::move(descRef), rtvDesc)
{}


BackBuffer::BackBuffer(BackBuffer&& other) :
	Texture2D(std::move(other)),
	m_RTV(std::move(other.m_RTV), std::shared_ptr<Texture2D>(this, [](Texture2D*) {}))
{}


BackBuffer& BackBuffer::operator=(BackBuffer&& other) {
	if (this == &other) {
		return *this;
	}

	m_resource = std::move(other.m_resource);
	m_resident = other.m_resident;
	m_RTV = RenderTargetView(std::move(other.m_RTV), std::shared_ptr<Texture2D>(this, [](Texture2D*) {}));

	return *this;
}


RenderTargetView& BackBuffer::GetView() {
	return m_RTV;
}


} // namespace gxeng
} // namespace inl
