#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include "HighLevelDescHeap.hpp"

namespace inl {
namespace gxeng {

namespace impl {
	class BasicHeap;
}

class MemoryManager;

//==================================
// Generic Resource

class GenericResource {
	friend class MemoryManager;
public:
	~GenericResource() noexcept;

	void* GetVirtualAddress() const;
	gxapi::ResourceDesc GetDescriptor() const;
	gxapi::DescriptorHandle GetViewHandle();

protected:
	GenericResource(TextureSpaceRef resourceView);
	GenericResource(const GenericResource&) = delete;
	GenericResource& operator=(GenericResource) = delete;

protected:
	gxapi::IResource* m_resource;
	impl::BasicHeap* m_resourceHeap;
	TextureSpaceRef m_resourceView;
};

//==================================


//==================================
// Vertex buffer, index buffer

class VertexBuffer : public GenericResource {
	friend class MemoryManager;
public:
	uint64_t GetSize() const;

protected:
	VertexBuffer(TextureSpaceRef resourceView);
};


using IndexBuffer = VertexBuffer;

//==================================


//==================================
// Shared Texture Properties

class GenericTextureBase : public GenericResource {
public:
	uint64_t GetWidth() const;
	gxapi::eFormat GetFormat() const;
protected:
	GenericTextureBase(TextureSpaceRef resourceView);
};

//==================================


//==================================
// Textures

class Texture1D : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint16_t GetArrayCount() const;

protected:
	Texture1D(TextureSpaceRef resourceView);
};


class Texture2D : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint64_t GetHeight() const;
	uint16_t GetArrayCount() const;

protected:
	Texture2D(TextureSpaceRef resourceView);
};


class Texture3D : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint64_t GetHeight() const;
	uint16_t GetDepth() const;

protected:
	Texture3D(TextureSpaceRef resourceView);
};


class TextureCube : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint64_t GetHeight() const;

protected:
	TextureCube(TextureSpaceRef resourceView);
};

//==================================

} // namespace gxeng
} // namespace inl
