
#include "GpuBuffer.hpp"

#include "../GraphicsApi_LL/ICommandList.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

#include "MemoryManager.hpp"
#include "ResourceHeap.hpp"

namespace inl {
namespace gxeng {

using namespace gxapi;

//==================================
//Generic Resource

GenericResource::~GenericResource() {
	m_resourceHeap->ReleaseUnderlying(this);
}


void* GenericResource::GetVirtualAddress() const {
	return m_resource->GetGPUAddress();
}


gxapi::ResourceDesc GenericResource::GetDescriptor() const {
	return m_resource->GetDesc();
}

gxapi::DescriptorHandle GenericResource::GetViewHandle() {
	return m_resourceView.Get();
}

GenericResource::GenericResource(TextureSpaceRef&& resourceView) :
	m_resourceView(std::move(resourceView))
{}


//==================================


uint64_t VertexBuffer::GetSize() const {
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
