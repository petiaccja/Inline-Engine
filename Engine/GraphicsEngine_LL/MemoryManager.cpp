
#include "MemoryManager.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "MemoryObject.hpp"

#include <algorithm>
#include <cassert>


namespace inl {
namespace gxeng {


MemoryManager::MemoryManager(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi),
	m_criticalHeap(graphicsApi),
	m_uploadHeap(graphicsApi),
	m_constBufferHeap(graphicsApi)
{}


void MemoryManager::LockResident(const std::vector<MemoryObject>& resources) {
	// Lock resident wants to modify contents but not the vector.
	auto& nonconst = const_cast<std::vector<MemoryObject>&>(resources);
	LockResident(nonconst.begin(), nonconst.end());
}


void MemoryManager::UnlockResident(const std::vector<MemoryObject>& resources) {
	auto& nonconst = const_cast<std::vector<MemoryObject>&>(resources);
	UnlockResident(nonconst.begin(), nonconst.end());
}


UploadManager& MemoryManager::GetUploadManager() {
	return m_uploadHeap;
}


VolatileConstBuffer MemoryManager::CreateVolatileConstBuffer(void* data, uint32_t size) {
	return m_constBufferHeap.CreateVolatileBuffer(data, size);
}


PersistentConstBuffer MemoryManager::CreatePersistentConstBuffer(void * data, uint32_t size) {
	return m_constBufferHeap.CreatePersistentBuffer(data, size);
}


VertexBuffer MemoryManager::CreateVertexBuffer(eResourceHeapType heap, size_t size) {
	MemoryObjDesc desc = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	VertexBuffer result(std::move(desc));
	return result;
}


IndexBuffer MemoryManager::CreateIndexBuffer(eResourceHeapType heap, size_t size, size_t indexCount) {
	MemoryObjDesc desc = AllocateResource(heap, gxapi::ResourceDesc::Buffer(size));

	IndexBuffer result(std::move(desc), indexCount);
	return result;
}


Texture1D MemoryManager::CreateTexture1D(eResourceHeapType heap, uint64_t width, gxapi::eFormat format, gxapi::eResourceFlags flags, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	MemoryObjDesc desc = AllocateResource(heap, gxapi::ResourceDesc::Texture1DArray(width, format, arraySize, flags));

	Texture1D result(std::move(desc));
	return result;
}


Texture2D MemoryManager::CreateTexture2D(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, gxapi::eResourceFlags flags, uint16_t arraySize) {
	if (arraySize < 1) {
		throw gxapi::InvalidArgument("\"count\" should not be at least one.");
	}

	MemoryObjDesc desc = AllocateResource(heap, gxapi::ResourceDesc::Texture2DArray(width, height, format, arraySize, flags));

	Texture2D result(std::move(desc));
	return result;
}


Texture3D MemoryManager::CreateTexture3D(eResourceHeapType heap, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format, gxapi::eResourceFlags flags) {
	MemoryObjDesc desc = AllocateResource(heap, gxapi::ResourceDesc::Texture3D(width, height, depth, format));

	Texture3D result(std::move(desc));
	return result;
}


TextureCube MemoryManager::CreateTextureCube(eResourceHeapType heap, uint64_t width, uint32_t height, gxapi::eFormat format, gxapi::eResourceFlags flags) {
	MemoryObjDesc desc = AllocateResource(heap, gxapi::ResourceDesc::CubeMap(width, height, format));

	TextureCube result(std::move(desc));
	return result;
}


MemoryObjDesc MemoryManager::AllocateResource(eResourceHeapType heap, const gxapi::ResourceDesc& desc) {

	gxapi::ClearValue* pClearValue = nullptr;

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
			throw std::runtime_error("Resource requested to be created is a depth stencil texture but its format can not be recognized while determining clear value format.");
		}
	}

	gxapi::ClearValue dsvClearValue(clearFormat, 1, 0);
	gxapi::ClearValue rtvClearValue(clearFormat, gxapi::ColorRGBA(0, 0, 0, 1));
	if (depthStencilTexture) {
		pClearValue = &dsvClearValue;
	}
	if (renderTargetTexture) {
		pClearValue = &rtvClearValue;
	}

	switch(heap) {
	case eResourceHeapType::CRITICAL: 
		return m_criticalHeap.Allocate(std::move(desc), pClearValue);
		break;
	default:
		assert(false);
	}

	return MemoryObjDesc();
}


} // namespace gxeng
} // namespace inl
