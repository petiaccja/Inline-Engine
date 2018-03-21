#pragma once

#include <GraphicsApi_LL/IResource.hpp>
#include <BaseLibrary/Exception/Exception.hpp>
#include "MemoryObject.hpp"


namespace inl::gxeng {


struct Texture1DDesc;
struct Texture3DDesc;
struct Texture2DDesc;



class BufferHeap {
	NotSupportedException DefaultEx() {
		return NotSupportedException{ "This heap cannot create the resource of requested type." };
	}
public:
	virtual ~BufferHeap() {}

	virtual VolatileConstBuffer CreateVolatileConstBuffer(const void* data, uint32_t size) { throw DefaultEx(); }
	virtual PersistentConstBuffer CreatePersistentConstBuffer(const void* data, uint32_t size) { throw DefaultEx(); }
	virtual VertexBuffer CreateVertexBuffer(size_t size) { throw DefaultEx(); }
	virtual IndexBuffer CreateIndexBuffer(size_t size, size_t indexCount) { throw DefaultEx(); }
	virtual Texture1D CreateTexture1D(const Texture1DDesc& desc, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE) { throw DefaultEx(); }
	virtual Texture2D CreateTexture2D(const Texture2DDesc& desc, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE) { throw DefaultEx(); }
	virtual Texture3D CreateTexture3D(const Texture3DDesc& desc, gxapi::eResourceFlags flags = gxapi::eResourceFlags::NONE) { throw DefaultEx(); }
};


} // namespace inl::gxeng