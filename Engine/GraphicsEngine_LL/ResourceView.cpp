#include "ResourceView.hpp"

#include "MemoryObject.hpp"
#include "HostDescHeap.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>


namespace inl {
namespace gxeng {


VertexBufferView::VertexBufferView(const VertexBuffer& resource, uint32_t stride, uint32_t size) :
	m_resource(resource),
	m_stride(stride),
	m_size(size)
{}


const VertexBuffer& VertexBufferView::GetResource() {
	return m_resource;
}


ConstBufferView::ConstBufferView(const VolatileConstBuffer& resource, CbvSrvUavHeap& heap) :
	ResourceViewBase(resource, &heap)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	heap.CreateCBV(desc, GetHandle());
}


ConstBufferView::ConstBufferView(const PersistentConstBuffer& resource, CbvSrvUavHeap& heap) :
	ResourceViewBase(resource, &heap)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	heap.CreateCBV(desc, GetHandle());
}


ConstBufferView::ConstBufferView(const VolatileConstBuffer & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi)
	: ResourceViewBase(resource, handle)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	gxapi->CreateConstantBufferView(desc, GetHandle());
}


ConstBufferView::ConstBufferView(const PersistentConstBuffer & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi)
	: ResourceViewBase(resource, handle)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	gxapi->CreateConstantBufferView(desc, GetHandle());
}


RenderTargetView2D::RenderTargetView2D(
	const Texture2D& resource,
	RTVHeap& heap, 
	gxapi::eFormat format,
	gxapi::RtvTexture2DArray desc
) :
	ResourceViewBase(resource, &heap)
{
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = format;
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	heap.Create(GetResource(), RTVdesc, GetHandle());
}

RenderTargetView2D::RenderTargetView2D(
	const Texture2D & resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi * gxapi,
	gxapi::eFormat format,
	gxapi::RtvTexture2DArray desc
) :
	ResourceViewBase(resource, handle)
{
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = format;
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	gxapi->CreateRenderTargetView(GetResource()._GetResourcePtr(), RTVdesc, GetHandle());
}

gxapi::RenderTargetViewDesc RenderTargetView2D::GetDescription() const {
	return m_desc;
}


DepthStencilView2D::DepthStencilView2D(
	const Texture2D& resource,
	DSVHeap& heap,
	gxapi::eFormat format,
	gxapi::DsvTexture2DArray desc
) :
	ResourceViewBase(resource, &heap)
{
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = format;
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	heap.Create(GetResource(), DSVdesc, GetHandle());
}

DepthStencilView2D::DepthStencilView2D(const Texture2D & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::DsvTexture2DArray desc)
	: ResourceViewBase(resource, handle)
{
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = format;
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	gxapi->CreateDepthStencilView(GetResource()._GetResourcePtr(), DSVdesc, GetHandle());
}



gxapi::DepthStencilViewDesc DepthStencilView2D::GetDescription() const {
	return m_desc;
}


BufferView::BufferView(
	const LinearBuffer& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc
) :
	ResourceViewBase(resource, &heap),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::BUFFER;
	fullSrvDesc.buffer = desc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());
}

BufferView::BufferView(
	const LinearBuffer& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi * gxapi,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc
) :
	ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::BUFFER;
	fullSrvDesc.buffer = desc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat BufferView::GetFormat() {
	return m_format;
}


const gxapi::SrvBuffer& BufferView::GetDescription() const {
	return m_srvDesc;
}


TextureView1D::TextureView1D(
	const Texture1D& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvTexture1DArray desc
) :
	ResourceViewBase(resource, &heap),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE1DARRAY;
	fullSrvDesc.tex1DArray = desc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());
}

TextureView1D::TextureView1D(
	const Texture1D & resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi * gxapi,
	gxapi::eFormat format,
	gxapi::SrvTexture1DArray desc
) :
	ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE1DARRAY;
	fullSrvDesc.tex1DArray = desc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat TextureView1D::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture1DArray& TextureView1D::GetDescription() const {
	return m_srvDesc;
}


TextureView2D::TextureView2D(
	const Texture2D& resource,
	CbvSrvUavHeap & heap,
	gxapi::eFormat format,
	gxapi::SrvTexture2DArray srvDesc
) :
	ResourceViewBase(resource, &heap),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE2DARRAY;
	fullSrvDesc.tex2DArray = srvDesc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());
}

TextureView2D::TextureView2D(
	const Texture2D & resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi * gxapi,
	gxapi::eFormat format,
	gxapi::SrvTexture2DArray srvDesc
) :
	ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE2DARRAY;
	fullSrvDesc.tex2DArray = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat TextureView2D::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture2DArray& TextureView2D::GetDescription() const {
	return m_srvDesc;
}


TextureView3D::TextureView3D(
	const Texture3D& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvTexture3D srvDesc
) :
	ResourceViewBase(resource, &heap),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE3D;
	fullSrvDesc.tex3D = srvDesc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());
}

TextureView3D::TextureView3D(
	const Texture3D & resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi * gxapi,
	gxapi::eFormat format,
	gxapi::SrvTexture3D srvDesc
) :
	ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE3D;
	fullSrvDesc.tex3D = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat TextureView3D::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture3D& TextureView3D::GetDescription() const {
	return m_srvDesc;
}


TextureViewCube::TextureViewCube(
	const TextureCube& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvTextureCube srvDesc
) :
	ResourceViewBase(resource, &heap),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURECUBE;
	fullSrvDesc.texCube = srvDesc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());
}

TextureViewCube::TextureViewCube(
	const TextureCube & resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi * gxapi,
	gxapi::eFormat format,
	gxapi::SrvTextureCube srvDesc
) :
	ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURECUBE;
	fullSrvDesc.texCube = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat TextureViewCube::GetFormat() {
	return m_format;
}


const gxapi::SrvTextureCube & TextureViewCube::GetDescription() const {
	return m_srvDesc;
}


} // namespace gxeng
} // namespace inl

