#include "ResourceView.hpp"

#include "GpuBuffer.hpp"

namespace inl {
namespace gxeng {


VertexBufferView::VertexBufferView(const std::shared_ptr<VertexBuffer>& resource, uint32_t stride, uint32_t size) :
	m_resource(resource),
	m_stride(stride),
	m_size(size)
{}


const std::shared_ptr<VertexBuffer>& VertexBufferView::GetResource() {
	return m_resource;
}


ConstBufferView::ConstBufferView(const std::shared_ptr<ConstBuffer>& resource, DescriptorReference&& descRef) :
	ResourceViewBase(resource, std::move(descRef))
{}


RenderTargetView::RenderTargetView(
	const std::shared_ptr<Texture2D>& resource,
	DescriptorReference && descRef,
	gxapi::RenderTargetViewDesc desc
):
	ResourceViewBase(resource, std::move(descRef)),
	m_desc(desc)
{}


RenderTargetView::RenderTargetView(
	RenderTargetView&& other,
	const std::shared_ptr<Texture2D>& resource
):
	ResourceViewBase(resource, std::move(*other.m_descRef)),
	m_desc(std::move(other.m_desc))
{
	other.m_resource.reset();
	other.m_descRef.reset();
}


RenderTargetView::RenderTargetView(
	const RenderTargetView& other,
	const std::shared_ptr<Texture2D>& resource
):
	ResourceViewBase(resource, other.m_descRef),
	m_desc(other.m_desc)
{}


gxapi::RenderTargetViewDesc RenderTargetView::GetDescription() const {
	return m_desc;
}


DepthStencilView::DepthStencilView(
	const std::shared_ptr<Texture2D>& resource,
	DescriptorReference && descRef,
	gxapi::DepthStencilViewDesc desc
):
	ResourceViewBase(resource, std::move(descRef)),
	m_desc(desc)
{}



gxapi::DepthStencilViewDesc DepthStencilView::GetDescription() const {
	return m_desc;
}


BufferSRV::BufferSRV(
	const std::shared_ptr<LinearBuffer>& resource,
	DescriptorReference&& desc,
	gxapi::eFormat format,
	gxapi::SrvBuffer srvDesc
):
	ShaderResourceView(resource, std::move(desc), format),
	m_srvDesc(srvDesc)
{}


const gxapi::SrvBuffer& BufferSRV::GetDescription() const {
	return m_srvDesc;
}


} // namespace gxeng
} // namespace inl

