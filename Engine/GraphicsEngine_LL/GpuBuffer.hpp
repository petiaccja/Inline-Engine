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
	friend class MemoryManager;
	friend class UploadHeap;
	friend class CopyCommandList;
public:
	GenericResource(GenericResource&&);
	GenericResource& operator=(GenericResource&&) noexcept;
	~GenericResource() noexcept;

	GenericResource(const GenericResource&) = delete;
	GenericResource& operator=(GenericResource) = delete;

	void* GetVirtualAddress() const;
	gxapi::ResourceDesc GetDescription() const;
	gxapi::DescriptorHandle GetViewHandle();

	/// <summary> Records the current state of the resource. Does not change resource state, only used for tracking it. </summary>
	void RecordState(unsigned subresource, gxapi::eResourceState newState);
	/// <summary> Records the state of all subresources. Does not change resource state, only used for tracking it. </summary>
	void RecordState(gxapi::eResourceState newState);
	/// <summary> Returns the current tracked state. </summary>
	gxapi::eResourceState ReadState(unsigned subresource) const;
protected:
	explicit GenericResource(DescriptorReference&& resourceView);

	void InitResourceStates(unsigned numSubresources, gxapi::eResourceState initialState);
protected:
	gxapi::IResource* m_resource;
	std::function<void(GenericResource*)> m_deleter;
	DescriptorReference m_resourceView;
	bool m_resident;
private:
	std::vector<gxapi::eResourceState> m_subresourceStates;
};

//==================================


//==================================
// Vertex buffer, index buffer

class LinearBuffer : public GenericResource {
	friend class MemoryManager;
public:
	LinearBuffer();

	uint64_t GetSize() const;
protected:
	using GenericResource::GenericResource;
};


using VertexBuffer = LinearBuffer;
using IndexBuffer = LinearBuffer;

//==================================


//==================================
// Shared Texture Properties

class GenericTextureBase : public GenericResource {
public:
	uint64_t GetWidth() const;
	gxapi::eFormat GetFormat() const;
protected:
	using GenericResource::GenericResource;
};

//==================================


//==================================
// Textures

class Texture1D : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint16_t GetArrayCount() const;

protected:
	using GenericTextureBase::GenericTextureBase;
};


class Texture2D : public GenericTextureBase {
	friend class MemoryManager;
	friend class BackBufferHeap;
public:
	uint64_t GetHeight() const;
	uint16_t GetArrayCount() const;

protected:
	using GenericTextureBase::GenericTextureBase;
};


class Texture3D : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint64_t GetHeight() const;
	uint16_t GetDepth() const;

protected:
	using GenericTextureBase::GenericTextureBase;
};


class TextureCube : public GenericTextureBase {
	friend class MemoryManager;
public:
	uint64_t GetHeight() const;

protected:
	using GenericTextureBase::GenericTextureBase;
};

//==================================

} // namespace gxeng
} // namespace inl
