#pragma once

//#include "ResourceView.hpp"

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include <functional>
#include <cassert>
#include <string>

namespace inl::gxeng {


class MemoryManager;
class RenderTargetView2D;


enum class eResourceHeap {
	UPLOAD = 1,
	CONSTANT,
	STREAMING,
	PIPELINE,
	CRITICAL,
	BACKBUFFER,
	INVALID,
};


class MemoryObject {
public:
	friend struct std::hash<MemoryObject>;

	using UniquePtr = std::unique_ptr<gxapi::IResource, std::function<void(const gxapi::IResource*)>>;
public:
	static bool PtrLess(const MemoryObject& lhs, const MemoryObject& rhs);
	static bool PtrGreater(const MemoryObject& lhs, const MemoryObject& rhs);
	static bool PtrEqual(const MemoryObject& lhs, const MemoryObject& rhs);

public:
	MemoryObject() = default;
	MemoryObject(UniquePtr resource, bool resident, eResourceHeap heap);
	virtual ~MemoryObject() = default;

	MemoryObject(const MemoryObject&) = default;
	MemoryObject(MemoryObject&&) = default;

	MemoryObject& operator=(const MemoryObject&) = default;
	MemoryObject& operator=(MemoryObject&&) = default;


	/// <summary> Returns true if and only if operands are referring to the same resource.
	/// Different MemoryObject instances are equal if one is the copy of the other. </summary>
	bool operator==(const MemoryObject&) const;

	/// <summary> Returns true if the operands do not refer to the same resource,
	///		or if one of them is empty. </summary>
	bool operator!=(const MemoryObject& rhs) const { return !(*this == rhs); }


	/// <summary> True if there is an actual GPU resource behind this object. False
	///		if this object has not been initialized yet. </summary>
	explicit operator bool() const {
		return (bool)m_contents;
	}

	/// <summary> True if there is an actual GPU resource behind this object. False
	///		if this object has not been initialized yet. </summary>
	bool HasObject() const {
		return (bool)m_contents;
	}
	
	/// <summary> Return the virtual address of the GPU resource behind this object. </summary>
	virtual void* GetVirtualAddress() const;

	/// <summary> Returns the description of the GPU resource as given by the underlying GxApi. </summary>
	gxapi::ResourceDesc GetDescription() const;

	/// <summary> Sets the name of the resource. Only for debug purposes. </summary>
	/// <remarks> You can easily identify the resources by their name in D3D debug output. </remarks>
	void SetName(const std::string& name);

	/// <summary> Sets the name of the resource. Only for debug purposes. </summary>
	/// <remarks> You can easily identify the resources by their name in D3D debug output. </remarks>
	void SetName(const char* name);

	/// <summary> Records the current state of the resource. Does not change resource state, only used for tracking it. </summary>
	void RecordState(unsigned subresource, gxapi::eResourceState newState);
	/// <summary> Records the state of all subresources. Does not change resource state, only used for tracking it. </summary>
	void RecordState(gxapi::eResourceState newState);
	/// <summary> Returns the current tracked state. </summary>
	gxapi::eResourceState ReadState(unsigned subresource) const;

	/// <summary> The total number of subresources, equals mipCount*arrayCount*planeCount. </summary>
	unsigned GetNumSubresources() const { return (unsigned)m_contents->resource->GetNumSubresources(); }

	/// <summary> Returns the number of mip levels. One if not applicable (e.g. for buffers). </summary>
	unsigned GetNumMiplevels() const { return (unsigned)m_contents->resource->GetNumMipLevels(); }


	eResourceHeap GetHeap() const { assert(m_contents->heap != eResourceHeap::INVALID); return m_contents->heap; }

	void _SetResident(bool value) noexcept;
	bool _GetResident() const noexcept;

	gxapi::IResource* _GetResourcePtr() const noexcept;

protected:
	void InitResourceStates(gxapi::eResourceState initialState);

	struct Contents {
		Contents() = default;
		Contents(UniquePtr&& resource, bool resident, eResourceHeap heap) : resource(std::move(resource)), resident(resident), heap(heap) {}
		UniquePtr resource;
		bool resident;
		eResourceHeap heap;
		std::vector<gxapi::eResourceState> subresourceStates;
		std::string name;
	};
	std::shared_ptr<Contents> m_contents;
};



//------------------------------------------------------------------------------
// Vertex buffer, index buffer
//------------------------------------------------------------------------------

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
	IndexBuffer() : m_indexCount(0) {}
	IndexBuffer(UniquePtr resource, bool resident, eResourceHeap heap, size_t indexCount);

	size_t GetIndexCount() const;

protected:
	size_t m_indexCount;
};


//------------------------------------------------------------------------------
// Const Buffers
//------------------------------------------------------------------------------


class ConstBuffer : public LinearBuffer {
public:
	void* GetVirtualAddress() const override;

	uint64_t GetSize() const;
	uint64_t GetDataSize() const;

protected:
	ConstBuffer(UniquePtr resource, bool resident, eResourceHeap heap, void* gpuVirtualPtr, uint32_t dataSize, uint32_t bufferSize);

protected:
	void* m_gpuVirtualPtr;

	// m_dataSize is the size of the data stored in this buffer
	uint32_t m_dataSize;

	// volatile const buffers are placed on pages where the data
	// is aligned and the allocation size fits the alignement
	// therefore the allocation might be bigger than the actual
	// data size. This allocation size is the m_bufferSize.
	uint32_t m_bufferSize;
};


class VolatileConstBuffer : public ConstBuffer {
public:
	VolatileConstBuffer(UniquePtr resource, bool resident, eResourceHeap heap, void* gpuVirtualPtr, uint32_t dataSize, uint32_t bufferSize);
};


class PersistentConstBuffer : public ConstBuffer {
public:
	PersistentConstBuffer(UniquePtr resource, bool resident, eResourceHeap heap, void* gpuVirtualPtr, uint32_t dataSize, uint32_t bufferSize);
};


//------------------------------------------------------------------------------
// Textures
//------------------------------------------------------------------------------


class Texture1D : public MemoryObject {
public:
	using MemoryObject::MemoryObject;

	uint64_t GetWidth() const;
	uint16_t GetArrayCount() const;
	uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex) const;
	gxapi::eFormat GetFormat() const;
};


class Texture2D : public MemoryObject {
public:
	using MemoryObject::MemoryObject;

	uint64_t GetWidth() const;
	uint32_t GetHeight() const;
	uint16_t GetArrayCount() const;
	uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex) const;
	gxapi::eFormat GetFormat() const;
};


class Texture3D : public MemoryObject {
public:
	using MemoryObject::MemoryObject;

	uint64_t GetWidth() const;
	uint64_t GetHeight() const;
	uint16_t GetDepth() const;
	uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t planeIndex) const;
	gxapi::eFormat GetFormat() const;
};


} // namespace inl::gxeng


namespace std {
template <>
struct hash<inl::gxeng::MemoryObject> {
	size_t operator()(const inl::gxeng::MemoryObject& obj) const {
		return reinterpret_cast<size_t>(obj.m_contents.get());
	}
};
}
