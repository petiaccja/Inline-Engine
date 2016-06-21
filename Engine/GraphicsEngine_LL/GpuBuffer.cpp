
#include "GpuBuffer.hpp"

#include "../GraphicsApi_LL/ICommandList.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

namespace inl {
namespace gxeng {

using namespace gxapi;

//==================================
//Generic Resource

void* GenericResource::GetVirtualAddress() const {
	return GetResource()->GetGPUAddress();
}


gxapi::ResourceDesc GenericResource::GetDescriptor() const {
	return GetResource()->GetDesc();
}


gxapi::IResource* GenericResource::GetResource() {
	return m_resource.get();
}

gxapi::IResource const* GenericResource::GetResource() const {
	return m_resource.get();
}


void GenericResource::ResetResource(gxapi::IGraphicsApi* graphicsApi, gxapi::ResourceDesc desc) {
	m_resource.reset(graphicsApi->CreateCommittedResource(
		HeapProperties(eHeapType::DEFAULT),
		eHeapFlags::NONE,
		desc,
		eResourceState::COMMON
	));
}

//==================================


VertexBuffer::VertexBuffer(gxapi::IGraphicsApi* graphicsApi, uint64_t size) {
	ResetResource(graphicsApi, ResourceDesc::Buffer(size));
}


uint64_t VertexBuffer::GetSize() const {
	return GetResource()->GetDesc().bufferDesc.sizeInBytes;
}


uint64_t GenericTextureBase::GetWidth() const {
	return GetResource()->GetDesc().textureDesc.width;
}


gxapi::eFormat GenericTextureBase::GetFormat() const {
	return GetResource()->GetDesc().textureDesc.format;
}


Texture1D::Texture1D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format, uint16_t elementCount) {
	if (elementCount == 0) {
		throw gxapi::InvalidArgument("\"count\" should not be zero.");
	}

	ResetResource(graphicsApi, ResourceDesc::Texture1DArray(width, format, elementCount));
}


uint16_t Texture1D::GetArrayCount() const {
	return GetResource()->GetDesc().textureDesc.depthOrArraySize;
}


Texture2D::Texture2D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t elementCount) {
	if (elementCount == 0) {
		throw gxapi::InvalidArgument("\"count\" should not be zero.");
	}

	ResetResource(graphicsApi, ResourceDesc::Texture2DArray(width, height, format, elementCount));
}


uint64_t Texture2D::GetHeight() const {
	return GetResource()->GetDesc().textureDesc.height;
}


uint16_t Texture2D::GetArrayCount() const {
	return GetResource()->GetDesc().textureDesc.depthOrArraySize;
}


Texture3D::Texture3D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	ResetResource(graphicsApi, ResourceDesc::Texture3D(width, height, depth, format));
}


uint64_t Texture3D::GetHeight() const {
	return GetResource()->GetDesc().textureDesc.height;
}


uint16_t Texture3D::GetDepth() const {
	return GetResource()->GetDesc().textureDesc.depthOrArraySize;
}


TextureCubeMap::TextureCubeMap(gxapi::IGraphicsApi * graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format) {
	ResetResource(graphicsApi, ResourceDesc::CubeMap(width, height, format));
}


uint64_t TextureCubeMap::GetHeight() const {
	return GetResource()->GetDesc().textureDesc.height;
}



} // namespace gxeng
} // namespace inl
