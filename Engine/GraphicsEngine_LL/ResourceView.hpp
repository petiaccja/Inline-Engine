#pragma once

#include "HostDescHeap.hpp"
#include "MemoryObject.hpp"

#include <GraphicsApi_LL/Common.hpp>

#include <cassert>
#include <iostream>
#include <utility>

namespace inl::gxeng {


class RTVHeap;
class DSVHeap;
class CbvSrvUavHeap;


template <typename ResourceT>
class ResourceViewBase {
	struct SharedState {
		ResourceT resource;
		IHostDescHeap* heap;
		size_t place;
		gxapi::DescriptorHandle handle;
		std::vector<uint32_t> subresourceList;
		SharedState() : resource{}, heap(nullptr), place(-1) {}
		SharedState(ResourceT resource, IHostDescHeap* heap, size_t place) : resource(std::move(resource)), heap(heap), place(place) {}
		SharedState(ResourceT resource, gxapi::DescriptorHandle handle) : resource(std::move(resource)), handle(handle), heap(nullptr), place(-1) {}
		~SharedState() {
			if (heap != nullptr) {
				heap->Deallocate(place);
			}
		}
	};

public:
	ResourceViewBase() = default;

	/// <summary> Returns the underlying resource. </summary>
	ResourceT& GetResource() {
		assert(operator bool());
		return m_state->resource;
	}
	/// <summary> Returns the underlying resource. </summary>
	const ResourceT& GetResource() const {
		assert(operator bool());
		return m_state->resource;
	}

	/// <summary> Returns the handle to the descriptor in the view's descriptor heap. </summary>
	gxapi::DescriptorHandle GetHandle() const {
		assert(operator bool());
		return m_state->handle;
	}

	/// <summary> Returns the list of subresources that this view references. </summary>
	const std::vector<uint32_t>& GetSubresourceList() const {
		assert(operator bool());
		assert(!m_state->subresourceList.empty()); // it means Peti forgot to call SetSubresourceList in of the view subclass ctors - pls add it yourself
		return m_state->subresourceList;
	}

	/// <summary> Check if this view has associated <see cref="MemoryObject"/> and descriptor. </summary>
	explicit operator bool() const {
		return (bool)m_state;
	}

protected:
	ResourceViewBase(const ResourceT& resource, IHostDescHeap* heap) {
		m_state = std::make_shared<SharedState>(resource, heap, heap->Allocate());
		m_state->handle = m_state->heap->At(m_state->place);
	}
	ResourceViewBase(const ResourceT& resource, gxapi::DescriptorHandle handle) {
		m_state = std::make_shared<SharedState>(resource, handle);
	}

	void SetSubresourceList(std::vector<uint32_t> list) {
		m_state->subresourceList = std::move(list);
	}

private:
	std::shared_ptr<SharedState> m_state;
};


class VertexBufferView {
public:
	VertexBufferView(const VertexBuffer& resource, uint32_t stride, uint32_t size);

	const VertexBuffer& GetResource();

	const std::vector<uint32_t>& GetSubresourceList() const {
		static const std::vector<uint32_t> list = std::initializer_list<uint32_t>{ gxapi::ALL_SUBRESOURCES };
		return list;
	}

protected:
	VertexBuffer m_resource;
	uint32_t m_stride;
	uint32_t m_size;
};


class ConstBufferView : public ResourceViewBase<ConstBuffer> {
public:
	ConstBufferView(const VolatileConstBuffer& resource, CbvSrvUavHeap& heap);
	ConstBufferView(const PersistentConstBuffer& resource, CbvSrvUavHeap& heap);
	ConstBufferView(const VolatileConstBuffer& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi);
	ConstBufferView(const PersistentConstBuffer& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi);
};


class RenderTargetView2D : public ResourceViewBase<Texture2D> {
public:
	RenderTargetView2D() = default;
	RenderTargetView2D(const Texture2D& resource, RTVHeap& heap, gxapi::eFormat format, gxapi::RtvTexture2DArray desc);
	RenderTargetView2D(const Texture2D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::RtvTexture2DArray desc);

	gxapi::RenderTargetViewDesc GetDescription() const;

protected:
	gxapi::RenderTargetViewDesc m_desc;
};


class DepthStencilView2D : public ResourceViewBase<Texture2D> {
public:
	DepthStencilView2D() = default;
	DepthStencilView2D(const Texture2D& resource, DSVHeap& heap, gxapi::eFormat format, gxapi::DsvTexture2DArray desc);
	DepthStencilView2D(const Texture2D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::DsvTexture2DArray desc);

	gxapi::DepthStencilViewDesc GetDescription() const;

protected:
	gxapi::DepthStencilViewDesc m_desc;
};



class BufferView : public ResourceViewBase<LinearBuffer> {
public:
	BufferView() = default;
	BufferView(const LinearBuffer& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvBuffer desc);
	BufferView(const LinearBuffer& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::SrvBuffer desc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvBuffer& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvBuffer m_srvDesc;
};


class TextureView1D : public ResourceViewBase<Texture1D> {
public:
	TextureView1D() = default;
	TextureView1D(const Texture1D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTexture1DArray desc);
	TextureView1D(const Texture1D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::SrvTexture1DArray desc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture1DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture1DArray m_srvDesc;
};


class TextureView2D : public ResourceViewBase<Texture2D> {
public:
	TextureView2D() = default;
	TextureView2D(const Texture2D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTexture2DArray desc);
	TextureView2D(const Texture2D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::SrvTexture2DArray desc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture2DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture2DArray m_srvDesc;
};


class TextureView3D : public ResourceViewBase<Texture3D> {
public:
	TextureView3D() = default;
	TextureView3D(const Texture3D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTexture3D desc);
	TextureView3D(const Texture3D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::SrvTexture3D desc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture3D& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture3D m_srvDesc;
};

/*
class TextureViewCube : public ResourceViewBase<TextureCube> {
public:
	TextureViewCube() = default;
	TextureViewCube(const TextureCube& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTextureCubeArray desc);
	TextureViewCube(const TextureCube& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::SrvTextureCubeArray desc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTextureCubeArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTextureCubeArray m_srvDesc;
};
*/
class TextureViewCube : public ResourceViewBase<Texture2D> {
public:
	TextureViewCube() = default;
	TextureViewCube(const Texture2D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTextureCubeArray desc);
	TextureViewCube(const Texture2D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::SrvTextureCubeArray desc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTextureCubeArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTextureCubeArray m_srvDesc;
};


class RWBufferView : public ResourceViewBase<LinearBuffer> {
public:
	RWBufferView() = default;
	RWBufferView(const LinearBuffer& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::UavBuffer desc);
	RWBufferView(const LinearBuffer& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::UavBuffer desc);

	gxapi::eFormat GetFormat();
	const gxapi::UavBuffer& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::UavBuffer m_uavDesc;
};


class RWTextureView1D : public ResourceViewBase<Texture1D> {
public:
	RWTextureView1D() = default;
	RWTextureView1D(const Texture1D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::UavTexture1DArray desc);
	RWTextureView1D(const Texture1D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::UavTexture1DArray desc);

	gxapi::eFormat GetFormat();
	const gxapi::UavTexture1DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::UavTexture1DArray m_uavDesc;
};


class RWTextureView2D : public ResourceViewBase<Texture2D> {
public:
	RWTextureView2D() = default;
	RWTextureView2D(const Texture2D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::UavTexture2DArray desc);
	RWTextureView2D(const Texture2D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::UavTexture2DArray desc);

	gxapi::eFormat GetFormat();
	const gxapi::UavTexture2DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::UavTexture2DArray m_uavDesc;
};


class RWTextureView3D : public ResourceViewBase<Texture3D> {
public:
	RWTextureView3D() = default;
	RWTextureView3D(const Texture3D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::UavTexture3D desc);
	RWTextureView3D(const Texture3D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::UavTexture3D desc);

	gxapi::eFormat GetFormat();
	const gxapi::UavTexture3D& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::UavTexture3D m_uavDesc;
};


} // namespace inl::gxeng
