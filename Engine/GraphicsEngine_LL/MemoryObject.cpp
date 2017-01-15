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



bool MemoryObject::PtrLess(const MemoryObject& lhs, const MemoryObject& rhs) {
	assert(lhs.m_contents);
	assert(rhs.m_contents);
	return lhs.m_contents.get() < rhs.m_contents.get();
}


bool MemoryObject::PtrGreater(const MemoryObject& lhs, const MemoryObject& rhs) {
	assert(lhs.m_contents);
	assert(rhs.m_contents);
	return lhs.m_contents.get() > rhs.m_contents.get();
}


bool MemoryObject::PtrEqual(const MemoryObject& lhs, const MemoryObject& rhs) {
	// "An empty shared_ptr may have a non-null stored pointer if the aliasing constructor was used to create it."
	// from: http://en.cppreference.com/w/cpp/memory/shared_ptr
	assert(lhs.m_contents);
	assert(rhs.m_contents);
	return lhs.m_contents.get() == rhs.m_contents.get();
}



MemoryObject::MemoryObject(MemoryObjDesc&& desc) :
	m_contents(new Contents{std::move(desc.resource), desc.resident, {}})
{
	InitResourceStates(eResourceState::COMMON);
}


bool MemoryObject::operator==(const MemoryObject& other) const {
	return PtrEqual(*this, other);
}


void* MemoryObject::GetVirtualAddress() const {
	assert(m_contents);
	return m_contents->resource->GetGPUAddress();
}


gxapi::ResourceDesc MemoryObject::GetDescription() const {
	assert(m_contents);
	return m_contents->resource->GetDesc();
}


void MemoryObject::_SetResident(bool value) noexcept {
	assert(m_contents);
	m_contents->resident = value;
}


bool MemoryObject::_GetResident() const noexcept {
	assert(m_contents);
	return m_contents->resident;
}


gxapi::IResource* MemoryObject::_GetResourcePtr() noexcept {
	assert(m_contents);
	return m_contents->resource.get();
}


const gxapi::IResource * MemoryObject::_GetResourcePtr() const noexcept {
	assert(m_contents);
	return m_contents->resource.get();
}


void MemoryObject::RecordState(unsigned subresource, gxapi::eResourceState newState) {
	assert(m_contents);
	assert(subresource < m_contents->subresourceStates.size());
	m_contents->subresourceStates[subresource] = newState;
}

void MemoryObject::RecordState(gxapi::eResourceState newState) {
	assert(m_contents);
	for (auto& state : m_contents->subresourceStates) {
		state = newState;
	}
}

gxapi::eResourceState MemoryObject::ReadState(unsigned subresource) const {
	assert(m_contents);
	assert(subresource < m_contents->subresourceStates.size());
	return m_contents->subresourceStates[subresource];
}

void MemoryObject::InitResourceStates(gxapi::eResourceState initialState) {
	assert(m_contents);
	gxapi::ResourceDesc desc = m_contents->resource->GetDesc();
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
	m_contents->subresourceStates.resize(numSubresources, initialState);
}

//==================================


uint64_t LinearBuffer::GetSize() const {
	return GetDescription().bufferDesc.sizeInBytes;
}

IndexBuffer::IndexBuffer(MemoryObjDesc&& desc, size_t indexCount) :
	LinearBuffer(std::move(desc)),
	m_indexCount(indexCount)
{}


size_t IndexBuffer::GetIndexCount() const {
	return m_indexCount;
}


//==================================


ConstBuffer::ConstBuffer(MemoryObjDesc&& desc, void* gpuVirtualPtr, uint32_t dataSize, uint32_t bufferSize) :
	LinearBuffer(std::move(desc)),
	m_gpuVirtualPtr(gpuVirtualPtr),
	m_dataSize(dataSize),
	m_bufferSize(bufferSize)
{}


void* ConstBuffer::GetVirtualAddress() const {
	return m_gpuVirtualPtr;
}


uint64_t ConstBuffer::GetSize() const {
	return m_bufferSize;
}


uint64_t ConstBuffer::GetDataSize() const {
	return m_dataSize;
}


VolatileConstBuffer::VolatileConstBuffer(MemoryObjDesc&& desc, void * gpuVirtualPtr, uint32_t dataSize, uint32_t bufferSize) :
	ConstBuffer(std::move(desc), gpuVirtualPtr, dataSize, bufferSize)
{}


PersistentConstBuffer::PersistentConstBuffer(MemoryObjDesc&& desc, void * gpuVirtualPtr, uint32_t dataSize, uint32_t bufferSize) :
	ConstBuffer(std::move(desc), gpuVirtualPtr, dataSize, bufferSize)
{}


//==================================


uint64_t Texture1D::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint16_t Texture1D::GetArrayCount() const {
	return GetDescription().textureDesc.depthOrArraySize;
}


gxapi::eFormat Texture1D::GetFormat() const {
	return GetDescription().textureDesc.format;
}


uint64_t Texture2D::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint64_t Texture2D::GetHeight() const {
	return GetDescription().textureDesc.height;
}


uint16_t Texture2D::GetArrayCount() const {
	return GetDescription().textureDesc.depthOrArraySize;
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
	return GetDescription().textureDesc.height;
}


uint16_t Texture3D::GetDepth() const {
	return GetDescription().textureDesc.depthOrArraySize;
}


gxapi::eFormat Texture3D::GetFormat() const {
	return GetDescription().textureDesc.format;
}


uint64_t TextureCube::GetWidth() const {
	return GetDescription().textureDesc.width;
}


uint64_t TextureCube::GetHeight() const {
	return GetDescription().textureDesc.height;
}


gxapi::eFormat TextureCube::GetFormat() const {
	return GetDescription().textureDesc.format;
}


} // namespace gxeng
} // namespace inl
