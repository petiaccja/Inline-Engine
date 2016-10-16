#include "ResourceView.hpp"

#include "GpuBuffer.hpp"

namespace inl {
namespace gxeng {


VertexBufferView::VertexBufferView(const std::shared_ptr<VertexBuffer>& resource, uint32_t stride, uint32_t size) :
	m_resource(resource),
	m_stride(stride),
	m_size(size)
{}


VertexBuffer& VertexBufferView::GetResource() {
	return *m_resource;
}


RenderTargetView::RenderTargetView(
	const std::shared_ptr<Texture2D>& resource,
	DescriptorReference && descRef,
	gxapi::RenderTargetViewDesc desc
):
	m_resource(resource),
	m_descRef(std::move(descRef)),
	m_desc(desc)
{}


gxapi::DescriptorHandle RenderTargetView::GetHandle() {
	return m_descRef.Get();
}

gxapi::RenderTargetViewDesc RenderTargetView::GetDescription() const {
	return m_desc;
}

const std::shared_ptr<Texture2D>& RenderTargetView::GetResource() {
	return m_resource;
}


DepthStencilView::DepthStencilView(
	const std::shared_ptr<Texture2D>& resource,
	DescriptorReference && descRef,
	gxapi::DepthStencilViewDesc desc
):
	m_resource(resource),
	m_descRef(std::move(descRef)),
	m_desc(desc)
{}


gxapi::DescriptorHandle DepthStencilView::GetHandle() {
	return m_descRef.Get();
}


gxapi::DepthStencilViewDesc DepthStencilView::GetDescription() const {
	return m_desc;
}


const std::shared_ptr<Texture2D>& DepthStencilView::GetResource() {
	return m_resource;
}


ShaderResourceView::ShaderResourceView(
	DescriptorReference&& desc,
	gxapi::eFormat format
):
	m_descRef(std::move(desc)),
	m_format(format)
{}


gxapi::DescriptorHandle ShaderResourceView::GetHandle() {
	return m_descRef.Get();
}


BufferSRV::BufferSRV(
	const std::shared_ptr<LinearBuffer>& resource,
	DescriptorReference&& desc,
	gxapi::eFormat format,
	gxapi::SrvBuffer srvDesc
):
	ShaderResourceView(std::move(desc), format),
	m_resource(resource),
	m_srvDesc(srvDesc)
{}


const gxapi::SrvBuffer& BufferSRV::GetDescription() const {
	return m_srvDesc;
}


LinearBuffer& BufferSRV::GetResource() {
	return *m_resource;
}


} // namespace gxeng
} // namespace inl
