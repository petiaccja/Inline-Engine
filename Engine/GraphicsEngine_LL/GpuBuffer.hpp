#pragma once

#include "ResourceView.hpp"
#include "HighLevelDescHeap.hpp"

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

namespace inl {
namespace gxeng {

class MemoryManager;

//==================================
// Generic Resource

class GenericResource {
public:
	using Deleter = std::function<void(gxapi::IResource*)>;

	GenericResource(gxapi::IResource* resource);
	GenericResource(gxapi::IResource* resource, const Deleter& deleter);
	virtual ~GenericResource() {}

	GenericResource(GenericResource&&);
	GenericResource& operator=(GenericResource&&);

	GenericResource(const GenericResource&) = delete;
	GenericResource& operator=(GenericResource) = delete;

	void* GetVirtualAddress() const;
	gxapi::ResourceDesc GetDescription() const;

	/// <summary> Records the current state of the resource. Does not change resource state, only used for tracking it. </summary>
	void RecordState(unsigned subresource, gxapi::eResourceState newState);
	/// <summary> Records the state of all subresources. Does not change resource state, only used for tracking it. </summary>
	void RecordState(gxapi::eResourceState newState);
	/// <summary> Returns the current tracked state. </summary>
	gxapi::eResourceState ReadState(unsigned subresource) const;

	void _SetResident(bool value) noexcept;
	bool _GetResident() const noexcept;

	gxapi::IResource* _GetResourcePtr() noexcept;
	const gxapi::IResource* _GetResourcePtr() const noexcept;
protected:
	void InitResourceStates(gxapi::eResourceState initialState);
protected:
	std::unique_ptr<gxapi::IResource, Deleter> m_resource;
	bool m_resident;
private:
	std::vector<gxapi::eResourceState> m_subresourceStates;
};

//==================================


//==================================
// Vertex buffer, index buffer

class LinearBuffer : public GenericResource {
public:
	uint64_t GetSize() const;
	using GenericResource::GenericResource;
};


class VertexBuffer : public LinearBuffer {
public:
	using LinearBuffer::LinearBuffer;
};


class IndexBuffer : public LinearBuffer {
public:
	IndexBuffer(gxapi::IResource* resource, size_t indexCount);
	IndexBuffer(gxapi::IResource* resource, const Deleter& deleter, size_t indexCount);

	size_t GetIndexCount() const;

protected:
	size_t m_indexCount;
};

//==================================


//==================================
// Const Buffers

class ConstBuffer : public LinearBuffer {
public:
	void* GetVirtualAddress() const;
	uint64_t GetSize() const;

protected:
	ConstBuffer(gxapi::IResource* resource, void* gpuVirtualPtr, uint32_t dataSize);

protected:
	void* m_gpuVirtualPtr;
	uint32_t m_dataSize;
};


class VolatileConstBuffer : public ConstBuffer {
public:
	VolatileConstBuffer(gxapi::IResource* resource, void* gpuVirtualPtr, uint32_t dataSize);
};


class PersistentConstBuffer : public ConstBuffer {
public:
	PersistentConstBuffer(gxapi::IResource* resource, void* gpuVirtualPtr, uint32_t dataSize);
};

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


class BackBuffer : public Texture2D {
public:
	BackBuffer(DescriptorReference&& descRef, gxapi::RenderTargetViewDesc desc, gxapi::IResource* resource);
	BackBuffer(BackBuffer&&);
	BackBuffer& operator=(BackBuffer&&);

	RenderTargetView& GetView();

protected:
	RenderTargetView m_RTV;
};

//==================================

} // namespace gxeng
} // namespace inl
