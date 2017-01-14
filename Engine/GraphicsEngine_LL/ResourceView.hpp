#pragma once

#include "HostDescHeap.hpp"
#include "MemoryObject.hpp"

#include <GraphicsApi_LL/Common.hpp>

#include <cassert>
#include <utility>
#include <iostream>

namespace inl {
namespace gxeng {


class RTVHeap;
class DSVHeap;
class CbvSrvUavHeap;


template <typename ResourceT>
class ResourceViewBase {
public:
	ResourceViewBase() : m_resource(nullptr) {};
	explicit ResourceViewBase(const ResourceT& resource) :
		m_resource(resource)
	{}

	ResourceT& GetResource() {
		return m_resource;
	}

	const ResourceT& GetResource() const {
		return m_resource;
	}

	gxapi::DescriptorHandle GetHandle() const {
		assert(m_descRef);
		return m_descRef->Get();
	}

protected:
	ResourceViewBase(const ResourceT& resource, const std::shared_ptr<DescriptorReference>& descRef) :
		m_resource(resource),
		m_descRef(descRef)
	{}

protected:
	ResourceT m_resource;
	std::shared_ptr<DescriptorReference> m_descRef;
};


class VertexBufferView {
public:
	VertexBufferView(const VertexBuffer& resource, uint32_t stride, uint32_t size);

	const VertexBuffer& GetResource();

protected:
	VertexBuffer m_resource;
	uint32_t m_stride;
	uint32_t m_size;
};


class ConstBufferView : public ResourceViewBase<ConstBuffer> {
public:
	ConstBufferView(const ConstBuffer& resource, size_t offset, size_t size, CbvSrvUavHeap& heap);
};


class RenderTargetView : public ResourceViewBase<Texture2D> {
public:
	RenderTargetView() = default;
	RenderTargetView(const Texture2D& resource, RTVHeap& heap, gxapi::RtvTexture2DArray desc);
	RenderTargetView(const Texture2D& resource, DescriptorReference&& handle, gxapi::RenderTargetViewDesc desc);
	
	gxapi::RenderTargetViewDesc GetDescription() const;

protected:
	gxapi::RenderTargetViewDesc m_desc;
};


class DepthStencilView : public ResourceViewBase<Texture2D> {
public:
	DepthStencilView() = default;
	DepthStencilView(const Texture2D& resource, DSVHeap& heap, gxapi::eFormat format, gxapi::DsvTexture2DArray desc);

	gxapi::DepthStencilViewDesc GetDescription() const;

protected:
	gxapi::DepthStencilViewDesc m_desc;
};


class BufferSRV : public ResourceViewBase<LinearBuffer> {
public:
	BufferSRV(const LinearBuffer& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvBuffer srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvBuffer& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvBuffer m_srvDesc;
};


class Texture1DSRV : public ResourceViewBase<Texture1D> {
public:
	Texture1DSRV() = default;
	Texture1DSRV(const Texture1D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTexture1DArray srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture1DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture1DArray m_srvDesc;
};


class Texture2DSRV : public ResourceViewBase<Texture2D> {
public:
	Texture2DSRV() = default;
	Texture2DSRV(const Texture2D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTexture2DArray srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture2DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture2DArray m_srvDesc;
};


class Texture3DSRV : public ResourceViewBase<Texture3D> {
public:
	Texture3DSRV() = default;
	Texture3DSRV(const Texture3D& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTexture3D srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture3D& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture3D m_srvDesc;
};


class TextureCubeSRV : public ResourceViewBase<TextureCube> {
public:
	TextureCubeSRV(const TextureCube& resource, CbvSrvUavHeap& heap, gxapi::eFormat format, gxapi::SrvTextureCube srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTextureCube& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTextureCube m_srvDesc;
};


} // namespace gxeng
} // namespace inl
