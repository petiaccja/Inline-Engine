#pragma once

#include "HighLevelDescHeap.hpp"

#include <utility>

namespace inl {
namespace gxeng {


class VertexBuffer;
class Texture2D;
class LinearBuffer;


class VertexBufferView {
public:
	VertexBufferView(const std::shared_ptr<VertexBuffer>& resource, uint32_t stride, uint32_t size);

	VertexBuffer& GetResource();

protected:
	std::shared_ptr<VertexBuffer> m_resource;
	uint32_t m_stride;
	uint32_t m_size;
};


class RenderTargetView {
public:
	RenderTargetView(const std::shared_ptr<Texture2D>& resource, DescriptorReference&& descRef, gxapi::RenderTargetViewDesc desc);
	
	gxapi::DescriptorHandle GetHandle();
	gxapi::RenderTargetViewDesc GetDescription() const;
	const std::shared_ptr<Texture2D>& GetResource();

protected:
	std::shared_ptr<Texture2D> m_resource;
	DescriptorReference m_descRef;
	gxapi::RenderTargetViewDesc m_desc;
};


class DepthStencilView {
public:
	DepthStencilView(const std::shared_ptr<Texture2D>& resource, DescriptorReference&& descRef, gxapi::DepthStencilViewDesc desc);

	gxapi::DescriptorHandle GetHandle();
	gxapi::DepthStencilViewDesc GetDescription() const;
	const std::shared_ptr<Texture2D>& GetResource();

protected:
	std::shared_ptr<Texture2D> m_resource;
	DescriptorReference m_descRef;
	gxapi::DepthStencilViewDesc m_desc;
};


class ShaderResourceView {
protected:
	ShaderResourceView(DescriptorReference&& desc, gxapi::eFormat format);

	gxapi::DescriptorHandle GetHandle();

protected:
	DescriptorReference m_descRef;
	gxapi::eFormat m_format;
};


class BufferSRV : public ShaderResourceView {
public:
	BufferSRV(const std::shared_ptr<LinearBuffer>& resource, DescriptorReference&& desc, gxapi::eFormat format, gxapi::SrvBuffer srvDesc);

	const gxapi::SrvBuffer& GetDescription() const;
	LinearBuffer& GetResource();

protected:
	std::shared_ptr<LinearBuffer> m_resource;
	gxapi::SrvBuffer m_srvDesc;
};


} // namespace gxeng
} // namespace inl
