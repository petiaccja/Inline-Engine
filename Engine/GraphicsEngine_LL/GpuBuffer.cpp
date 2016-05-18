
#include "GpuBuffer.hpp"

#include "../GraphicsApi_LL/IGraphicsCommandList.hpp"

namespace inl {
namespace gxeng {

using namespace gxapi;

//==================================
//Generic Resource

gxapi::IResource* GenericBuffer::GetResource() {
	return m_resource.get();
}


void GenericBuffer::ResetResource(gxapi::IGraphicsApi* graphicsApi, gxapi::ResourceDesc desc) {
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


Texture1D::Texture1D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format) {
	ResetResource(graphicsApi, ResourceDesc::Texture1D(width, format));
}


Texture1DArray::Texture1DArray(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format, uint16_t count) {
	ResetResource(graphicsApi, ResourceDesc::Texture1DArray(width, format, count));
}


Texture2D::Texture2D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format) {
	ResetResource(graphicsApi, ResourceDesc::Texture2D(width, height, format));
}

Texture2DArray::Texture2DArray(gxapi::IGraphicsApi * graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t count) {
	ResetResource(graphicsApi, ResourceDesc::Texture2DArray(width, height, format, count));
}


Texture3D::Texture3D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format) {
	ResetResource(graphicsApi, ResourceDesc::Texture3D(width, height, depth, format));
}


TextureCubeMap::TextureCubeMap(gxapi::IGraphicsApi * graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format) {
	ResetResource(graphicsApi, ResourceDesc::CubeMap(width, height, format));
}


} // namespace gxeng
} // namespace inl
