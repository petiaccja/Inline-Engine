#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include "ResourceHeap.hpp"
#include "HighLevelDescHeap.hpp"

namespace inl {
namespace gxeng {

class MemoryManager;

//==================================
// Generic Resource

class GenericResource {
public:
	using Deleter = std::function<void(gxapi::IResource*)>;

	GenericResource(DescriptorReference&& resourceView, gxapi::IResource* resource);
	GenericResource(DescriptorReference&& resourceView, gxapi::IResource* resource, const Deleter& deleter);

	GenericResource(GenericResource&&);
	GenericResource& operator=(GenericResource&&);

	GenericResource(const GenericResource&) = delete;
	GenericResource& operator=(GenericResource) = delete;

	void* GetVirtualAddress() const;
	gxapi::ResourceDesc GetDescription() const;
	gxapi::DescriptorHandle GetHandle();

	void _SetResident(bool value) noexcept;
	bool _GetResident() const noexcept;

	gxapi::IResource* _GetResourcePtr() noexcept;
	const gxapi::IResource* _GetResourcePtr() const noexcept;

protected:
	DescriptorReference m_resourceView;
	Deleter m_deleter;
	std::unique_ptr<gxapi::IResource, Deleter&> m_resource;
	bool m_resident;
};

//==================================


//==================================
// Vertex buffer, index buffer

class LinearBuffer : public GenericResource {
public:
	using GenericResource::GenericResource;

	uint64_t GetSize() const;
};


using VertexBuffer = LinearBuffer;
using IndexBuffer = LinearBuffer;

//==================================


//==================================
// Shared Texture Properties

class GenericTextureBase : public GenericResource {
public:
	using GenericResource::GenericResource;

	uint64_t GetWidth() const;
	gxapi::eFormat GetFormat() const;
};

//==================================


//==================================
// Textures

class Texture1D : public GenericTextureBase {
public:
	using GenericTextureBase::GenericTextureBase;

	uint16_t GetArrayCount() const;
};


class Texture2D : public GenericTextureBase {
public:
	using GenericTextureBase::GenericTextureBase;

	uint64_t GetHeight() const;
	uint16_t GetArrayCount() const;
};


class Texture3D : public GenericTextureBase {
public:
	using GenericTextureBase::GenericTextureBase;

	uint64_t GetHeight() const;
	uint16_t GetDepth() const;
};


class TextureCube : public GenericTextureBase {
public:
	using GenericTextureBase::GenericTextureBase;

	uint64_t GetHeight() const;
};

//==================================

} // namespace gxeng
} // namespace inl
