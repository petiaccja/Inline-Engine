#include "ResourceView.hpp"

#include "GpuBuffer.hpp"
#include "HighLevelDescHeap.hpp"

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


ConstBufferView::ConstBufferView(const std::shared_ptr<VolatileConstBuffer>& resource, PersistentResViewHeap& heap) :
	ResourceViewBase(resource)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource->GetVirtualAddress();
	desc.sizeInBytes = resource->GetSize();

	m_descRef.reset(new DescriptorReference(heap.CreateCBV(desc)));
}


ConstBufferView::ConstBufferView(const std::shared_ptr<PersistentConstBuffer>& resource, PersistentResViewHeap& heap) :
	ResourceViewBase(resource)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource->GetVirtualAddress();
	desc.sizeInBytes = resource->GetSize();

	m_descRef.reset(new DescriptorReference(heap.CreateCBV(desc)));
}


RenderTargetView::RenderTargetView(
	const std::shared_ptr<Texture2D>& resource,
	RTVHeap& heap,
	gxapi::RtvTexture2DArray desc
):
	ResourceViewBase(resource)
{
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = resource->GetFormat();
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	m_descRef.reset(new DescriptorReference(heap.Create(*resource, RTVdesc)));
}


RenderTargetView::RenderTargetView(
	const std::shared_ptr<Texture2D>& resource,
	DescriptorReference&& handle,
	gxapi::RenderTargetViewDesc desc
):
	ResourceViewBase(resource),
	m_desc(desc)
{
	m_descRef.reset(new DescriptorReference(std::move(handle)));
}


RenderTargetView::RenderTargetView(
	RenderTargetView&& other,
	const std::shared_ptr<Texture2D>& resource
):
	ResourceViewBase(resource),
	m_desc(std::move(other.m_desc))
{
	m_descRef = std::move(other.m_descRef);

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
	DSVHeap& heap,
	gxapi::DsvTexture2DArray desc
):
	ResourceViewBase(resource)
{
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = resource->GetFormat();
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	m_descRef.reset(new DescriptorReference(heap.Create(*resource, DSVdesc)));
}



gxapi::DepthStencilViewDesc DepthStencilView::GetDescription() const {
	return m_desc;
}


BufferSRV::BufferSRV(
	const std::shared_ptr<LinearBuffer>& resource,
	PersistentResViewHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc
):
	ResourceViewBase(resource),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc SRVdesc;
	SRVdesc.format = format;
	SRVdesc.dimension = gxapi::eSrvDimension::BUFFER;
	SRVdesc.buffer = desc;

	m_descRef.reset(new DescriptorReference(heap.CreateSRV(*resource, SRVdesc)));
}


gxapi::eFormat BufferSRV::GetFormat() {
	return m_format;
}


const gxapi::SrvBuffer& BufferSRV::GetDescription() const {
	return m_srvDesc;
}


} // namespace gxeng
} // namespace inl

