#pragma once

#include <GraphicsApi_LL/Common.hpp>

#include <cassert>
#include <utility>
#include <iostream>

namespace inl {
namespace gxeng {


class DescriptorReference;
class RTVHeap;
class DSVHeap;
class PersistentResViewHeap;
class VertexBuffer;
class ConstBuffer;
class VolatileConstBuffer;
class PersistentConstBuffer;
class LinearBuffer;
class Texture1D;
class Texture2D;
class Texture3D;
class TextureCube;


template <typename ResourceT>
class ResourceViewBase {
public:
	ResourceViewBase() = default;
	ResourceViewBase(const std::shared_ptr<ResourceT>& resource) :
		m_resource(resource)
	{}

	const std::shared_ptr<ResourceT>& GetResource() const {
		return m_resource;
	}

	gxapi::DescriptorHandle GetHandle() const {
		assert(m_descRef);
		return m_descRef->Get();
	}

protected:
	ResourceViewBase(const std::shared_ptr<ResourceT>& resource, const std::shared_ptr<DescriptorReference>& descRef) :
		m_resource(resource),
		m_descRef(descRef)
	{}

protected:
	std::shared_ptr<ResourceT> m_resource;
	std::shared_ptr<DescriptorReference> m_descRef;
};


class VertexBufferView {
public:
	VertexBufferView(const std::shared_ptr<VertexBuffer>& resource, uint32_t stride, uint32_t size);

	const std::shared_ptr<VertexBuffer>& GetResource();

protected:
	std::shared_ptr<VertexBuffer> m_resource;
	uint32_t m_stride;
	uint32_t m_size;
};


class ConstBufferView : public ResourceViewBase<ConstBuffer> {
public:
	ConstBufferView(const std::shared_ptr<VolatileConstBuffer>& resource, PersistentResViewHeap& heap);
	ConstBufferView(const std::shared_ptr<PersistentConstBuffer>& resource, PersistentResViewHeap& heap);
};


class RenderTargetView : public ResourceViewBase<Texture2D> {
public:
	RenderTargetView() = default;
	RenderTargetView(const std::shared_ptr<Texture2D>& resource, RTVHeap& heap, gxapi::RtvTexture2DArray desc);
	RenderTargetView(const std::shared_ptr<Texture2D>& resource, DescriptorReference&& handle, gxapi::RenderTargetViewDesc desc);
	RenderTargetView(RenderTargetView&& other, const std::shared_ptr<Texture2D>& resource);
	RenderTargetView(const RenderTargetView& other, const std::shared_ptr<Texture2D>& resource);
	
	gxapi::RenderTargetViewDesc GetDescription() const;

protected:
	gxapi::RenderTargetViewDesc m_desc;
};


class DepthStencilView : public ResourceViewBase<Texture2D> {
public:
	DepthStencilView() = default;
	DepthStencilView(const std::shared_ptr<Texture2D>& resource, DSVHeap& heap, gxapi::DsvTexture2DArray desc);

	gxapi::DepthStencilViewDesc GetDescription() const;

protected:
	gxapi::DepthStencilViewDesc m_desc;
};


class BufferSRV : public ResourceViewBase<LinearBuffer> {
public:
	BufferSRV(const std::shared_ptr<LinearBuffer>& resource, PersistentResViewHeap& heap, gxapi::eFormat format, gxapi::SrvBuffer srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvBuffer& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvBuffer m_srvDesc;
};


class Texture1DSRV : public ResourceViewBase<Texture1D> {
public:
	Texture1DSRV() = default;
	Texture1DSRV(const std::shared_ptr<Texture1D>& resource, PersistentResViewHeap& heap, gxapi::eFormat format, gxapi::SrvTexture1DArray srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture1DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture1DArray m_srvDesc;
};


class Texture2DSRV : public ResourceViewBase<Texture2D> {
public:
	Texture2DSRV() = default;
	Texture2DSRV(const std::shared_ptr<Texture2D>& resource, PersistentResViewHeap& heap, gxapi::eFormat format, gxapi::SrvTexture2DArray srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture2DArray& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture2DArray m_srvDesc;
};


class Texture3DSRV : public ResourceViewBase<Texture3D> {
public:
	Texture3DSRV() = default;
	Texture3DSRV(const std::shared_ptr<Texture3D>& resource, PersistentResViewHeap& heap, gxapi::eFormat format, gxapi::SrvTexture3D srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTexture3D& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTexture3D m_srvDesc;
};


class TextureCubeSRV : public ResourceViewBase<TextureCube> {
public:
	TextureCubeSRV(const std::shared_ptr<TextureCube>& resource, PersistentResViewHeap& heap, gxapi::eFormat format, gxapi::SrvTextureCube srvDesc);

	gxapi::eFormat GetFormat();
	const gxapi::SrvTextureCube& GetDescription() const;

protected:
	gxapi::eFormat m_format;
	gxapi::SrvTextureCube m_srvDesc;
};


} // namespace gxeng
} // namespace inl
