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


ConstBufferView::ConstBufferView(const VolatileConstBuffer& resource, PersistentResViewHeap& heap) :
	ResourceViewBase(resource, &heap)
{
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	heap.CreateCBV(desc, GetHandle());
}


ConstBufferView::ConstBufferView(const PersistentConstBuffer& resource, PersistentResViewHeap& heap) :
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


RenderTargetView::RenderTargetView(const Texture2D& resource,
								   RTVHeap& heap, 
								   gxapi::eFormat format,
								   gxapi::RtvTexture2DArray desc)
	:
	ResourceViewBase(resource, &heap)
{
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = format;
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	heap.Create(GetResource(), RTVdesc, GetHandle());
}

RenderTargetView::RenderTargetView(const Texture2D & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::RtvTexture2DArray desc)
	: ResourceViewBase(resource, handle)
{
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = format;
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	gxapi->CreateRenderTargetView(GetResource()._GetResourcePtr(), RTVdesc, GetHandle());
}

gxapi::RenderTargetViewDesc RenderTargetView::GetDescription() const {
	return m_desc;
}


DepthStencilView::DepthStencilView(const Texture2D& resource,
								   DSVHeap& heap,
								   gxapi::eFormat format,
								   gxapi::DsvTexture2DArray desc)
	:
	ResourceViewBase(resource, &heap)
{
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = format;
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	heap.Create(GetResource(), DSVdesc, GetHandle());
}

DepthStencilView::DepthStencilView(const Texture2D & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::DsvTexture2DArray desc)
	: ResourceViewBase(resource, handle)
{
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = format;
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	gxapi->CreateDepthStencilView(GetResource()._GetResourcePtr(), DSVdesc, GetHandle());
}



gxapi::DepthStencilViewDesc DepthStencilView::GetDescription() const {
	return m_desc;
}


BufferSRV::BufferSRV(const LinearBuffer& resource,
					 PersistentResViewHeap& heap,
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

BufferSRV::BufferSRV(const LinearBuffer & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::SrvBuffer desc)
	: ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::BUFFER;
	fullSrvDesc.buffer = desc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat BufferSRV::GetFormat() {
	return m_format;
}


const gxapi::SrvBuffer& BufferSRV::GetDescription() const {
	return m_srvDesc;
}


Texture1DSRV::Texture1DSRV(
	const Texture1D& resource,
	PersistentResViewHeap& heap,
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

Texture1DSRV::Texture1DSRV(const Texture1D & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::SrvTexture1DArray desc)
	: ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(desc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE1DARRAY;
	fullSrvDesc.tex1DArray = desc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat Texture1DSRV::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture1DArray& Texture1DSRV::GetDescription() const {
	return m_srvDesc;
}


Texture2DSRV::Texture2DSRV(
	const Texture2D& resource,
	PersistentResViewHeap & heap,
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

Texture2DSRV::Texture2DSRV(const Texture2D & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::SrvTexture2DArray srvDesc)
	: ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE2DARRAY;
	fullSrvDesc.tex2DArray = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat Texture2DSRV::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture2DArray& Texture2DSRV::GetDescription() const {
	return m_srvDesc;
}


Texture3DSRV::Texture3DSRV(
	const Texture3D& resource,
	PersistentResViewHeap& heap,
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

Texture3DSRV::Texture3DSRV(const Texture3D & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::SrvTexture3D srvDesc)
	: ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE3D;
	fullSrvDesc.tex3D = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat Texture3DSRV::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture3D& Texture3DSRV::GetDescription() const {
	return m_srvDesc;
}


TextureCubeSRV::TextureCubeSRV(
	const TextureCube& resource,
	PersistentResViewHeap& heap,
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

TextureCubeSRV::TextureCubeSRV(const TextureCube & resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi * gxapi, gxapi::eFormat format, gxapi::SrvTextureCube srvDesc)
	: ResourceViewBase(resource, handle),
	m_format(format),
	m_srvDesc(srvDesc)
{
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURECUBE;
	fullSrvDesc.texCube = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());
}


gxapi::eFormat TextureCubeSRV::GetFormat() {
	return m_format;
}


const gxapi::SrvTextureCube & TextureCubeSRV::GetDescription() const {
	return m_srvDesc;
}


} // namespace gxeng
} // namespace inl

