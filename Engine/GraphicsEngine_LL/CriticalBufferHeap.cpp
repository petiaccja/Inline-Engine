#include "CriticalBufferHeap.hpp"

#include "MemoryObject.hpp"
#include "CopyCommandList.hpp"
#include "MemoryManager.hpp"

#include <iostream>


namespace inl {
namespace gxeng {
namespace impl {



CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


CriticalBufferHeap::UniquePtr CriticalBufferHeap::Allocate(gxapi::ResourceDesc desc, gxapi::ClearValue* clearValue) {
	UniquePtr resource{
		m_graphicsApi->CreateCommittedResource(
			gxapi::HeapProperties(gxapi::eHeapType::DEFAULT, gxapi::eCpuPageProperty::UNKNOWN, gxapi::eMemoryPool::UNKNOWN),
			gxapi::eHeapFlags::NONE,
			desc,
			gxapi::eResourceState::COMMON,
			clearValue
		),
		std::default_delete<const gxapi::IResource>()
	};

	return resource;
}

gxapi::ClearValue CriticalBufferHeap::DetermineClearValue(const gxapi::ResourceDesc& desc) {
	bool depthStencilTexture = (desc.type == gxapi::eResourceType::TEXTURE) && (desc.textureDesc.flags & gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL);
	bool renderTargetTexture = (desc.type == gxapi::eResourceType::TEXTURE) && (desc.textureDesc.flags & gxapi::eResourceFlags::ALLOW_RENDER_TARGET);

	using gxapi::eFormat;
	gxapi::eFormat clearFormat = desc.textureDesc.format;
	if (depthStencilTexture) {
		switch (desc.textureDesc.format) {
		case eFormat::D16_UNORM:
		case eFormat::D24_UNORM_S8_UINT:
		case eFormat::D32_FLOAT:
		case eFormat::D32_FLOAT_S8X24_UINT:
			break; // just leave the format as it is
		case eFormat::R32_TYPELESS:
			clearFormat = eFormat::D32_FLOAT;
			break;
		case eFormat::R32G8X24_TYPELESS:
			clearFormat = eFormat::D32_FLOAT_S8X24_UINT;
			break;
		default:
			assert(false);
			// I know I know... this exception message is horribe
			throw InvalidArgumentException("Resource requested to be created is a depth stencil texture but its format can not be recognized while determining clear value format.");
		}
	}

	gxapi::ClearValue dsvClearValue(clearFormat, 1, 0);
	gxapi::ClearValue rtvClearValue(clearFormat, gxapi::ColorRGBA(0, 0, 0, 1));
	if (depthStencilTexture) {
		return dsvClearValue;
	}
	if (renderTargetTexture) {
		return rtvClearValue;
	}

	return gxapi::ClearValue{ gxapi::eFormat::UNKNOWN, 0,0 };
}


VertexBuffer CriticalBufferHeap::CreateVertexBuffer(size_t size) {
	auto apiDesc = gxapi::ResourceDesc::Buffer(size);
	gxapi::ClearValue clearValue = DetermineClearValue(apiDesc);
	auto resource = Allocate(apiDesc, clearValue.format != gxapi::eFormat::UNKNOWN ? &clearValue : nullptr);

	return VertexBuffer(std::move(resource), true, eResourceHeap::CRITICAL);
}


IndexBuffer CriticalBufferHeap::CreateIndexBuffer(size_t size, size_t indexCount) {
	auto apiDesc = gxapi::ResourceDesc::Buffer(size);
	gxapi::ClearValue clearValue = DetermineClearValue(apiDesc);
	auto resource = Allocate(apiDesc, clearValue.format != gxapi::eFormat::UNKNOWN ? &clearValue : nullptr);

	return IndexBuffer(std::move(resource), true, eResourceHeap::CRITICAL, indexCount);
}


Texture1D CriticalBufferHeap::CreateTexture1D(const Texture1DDesc& desc, gxapi::eResourceFlags flags) {
	auto apiDesc = gxapi::ResourceDesc::Texture1DArray(desc.width, desc.format, desc.arraySize, flags, desc.mipLevels);
	gxapi::ClearValue clearValue = DetermineClearValue(apiDesc);
	auto resource = Allocate(apiDesc, clearValue.format != gxapi::eFormat::UNKNOWN ? &clearValue : nullptr);

	return Texture1D(std::move(resource), true, eResourceHeap::CRITICAL);
}


Texture2D CriticalBufferHeap::CreateTexture2D(const Texture2DDesc& desc, gxapi::eResourceFlags flags) {
	auto apiDesc = gxapi::ResourceDesc::Texture2DArray(desc.width, desc.height, desc.format, desc.arraySize, flags, desc.mipLevels);
	gxapi::ClearValue clearValue = DetermineClearValue(apiDesc);
	auto resource = Allocate(apiDesc, clearValue.format != gxapi::eFormat::UNKNOWN ? &clearValue : nullptr);

	return Texture2D(std::move(resource), true, eResourceHeap::CRITICAL);
}


Texture3D CriticalBufferHeap::CreateTexture3D(const Texture3DDesc& desc, gxapi::eResourceFlags flags) {
	auto apiDesc = gxapi::ResourceDesc::Texture3D(desc.width, desc.height, desc.depth, desc.format, flags, desc.mipLevels);
	gxapi::ClearValue clearValue = DetermineClearValue(apiDesc);
	auto resource = Allocate(apiDesc, clearValue.format != gxapi::eFormat::UNKNOWN ? &clearValue : nullptr);

	return Texture3D(std::move(resource), true, eResourceHeap::CRITICAL);
}


} // namespace impl
} // namespace gxeng
} // namespace inl
