#pragma once

#include "ResourceView.hpp"

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include <functional>

namespace inl {
namespace gxeng {


class MemoryManager;


//==================================

struct MemoryObjectDescriptor {
	gxapi::IResource* resource;
	bool resident;

	// Deleter must contain a valid callable. (std::default_delete is acceppted)
	std::function<void(gxapi::IResource*)> deleter; 
};

class MemoryObject {
public:
	using Deleter = std::function<void(gxapi::IResource*)>;

	MemoryObject(MemoryObjectDescriptor desc);
	virtual ~MemoryObject() {}

	MemoryObject(MemoryObject&&);
	MemoryObject& operator=(MemoryObject&&);

	MemoryObject(const MemoryObject&) = delete;
	MemoryObject& operator=(MemoryObject) = delete;

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

class LinearBuffer : public MemoryObject {
public:
	uint64_t GetSize() const;
	using MemoryObject::MemoryObject;
};


class VertexBuffer : public LinearBuffer {
public:
	using LinearBuffer::LinearBuffer;
};


class IndexBuffer : public LinearBuffer {
public:
	IndexBuffer(MemoryObjectDescriptor desc, size_t indexCount);

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
	ConstBuffer(MemoryObjectDescriptor desc, void* gpuVirtualPtr, uint32_t dataSize);

protected:
	void* m_gpuVirtualPtr;
	uint32_t m_dataSize;
};


class VolatileConstBuffer : public ConstBuffer {
public:
	VolatileConstBuffer(MemoryObjectDescriptor desc, void* gpuVirtualPtr, uint32_t dataSize);
};


class PersistentConstBuffer : public ConstBuffer {
public:
	PersistentConstBuffer(MemoryObjectDescriptor desc, void* gpuVirtualPtr, uint32_t dataSize);
};

//==================================


//==================================
// Shared Texture Properties

class GenericTextureBase : public MemoryObject {
public:
	using MemoryObject::MemoryObject;

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
	uint32_t GetSubresourceIndex(uint32_t arrayIndex, uint32_t mipLevel) const;
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
	BackBuffer(DescriptorReference&& descRef, gxapi::RenderTargetViewDesc rtvDesc, MemoryObjectDescriptor desc);
	BackBuffer(BackBuffer&&);
	BackBuffer& operator=(BackBuffer&&);

	RenderTargetView& GetView();

protected:
	RenderTargetView m_RTV;
};

//==================================

} // namespace gxeng
} // namespace inl
