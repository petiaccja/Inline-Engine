#include "ResourceView.hpp"

#include "HostDescHeap.hpp"
#include "MemoryObject.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>


namespace inl::gxeng {


static std::vector<uint32_t> CalcSubresourceList(const Texture2D&, const gxapi::RtvTexture2DArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture2D&, const gxapi::DsvTexture2DArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture1D&, const gxapi::SrvTexture1DArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture2D&, const gxapi::SrvTexture2DArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture3D&, const gxapi::SrvTexture3D&);
static std::vector<uint32_t> CalcSubresourceList(const Texture2D&, const gxapi::SrvTextureCubeArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture1D&, const gxapi::UavTexture1DArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture2D&, const gxapi::UavTexture2DArray&);
static std::vector<uint32_t> CalcSubresourceList(const Texture3D&, const gxapi::UavTexture3D&);



VertexBufferView::VertexBufferView(const VertexBuffer& resource, uint32_t stride, uint32_t size) : m_resource(resource),
																								   m_stride(stride),
																								   m_size(size) {}


const VertexBuffer& VertexBufferView::GetResource() {
	return m_resource;
}


ConstBufferView::ConstBufferView(const VolatileConstBuffer& resource, CbvSrvUavHeap& heap) : ResourceViewBase(resource, &heap) {
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	heap.CreateCBV(desc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
}


ConstBufferView::ConstBufferView(const PersistentConstBuffer& resource, CbvSrvUavHeap& heap) : ResourceViewBase(resource, &heap) {
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	heap.CreateCBV(desc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
}


ConstBufferView::ConstBufferView(const VolatileConstBuffer& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi)
	: ResourceViewBase(resource, handle) {
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	gxapi->CreateConstantBufferView(desc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
}


ConstBufferView::ConstBufferView(const PersistentConstBuffer& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi)
	: ResourceViewBase(resource, handle) {
	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource.GetVirtualAddress();
	desc.sizeInBytes = resource.GetSize();

	gxapi->CreateConstantBufferView(desc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
}


RenderTargetView2D::RenderTargetView2D(
	const Texture2D& resource,
	RTVHeap& heap,
	gxapi::eFormat format,
	gxapi::RtvTexture2DArray desc) : ResourceViewBase(resource, &heap) {
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = format;
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	heap.Create(GetResource(), RTVdesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

RenderTargetView2D::RenderTargetView2D(
	const Texture2D& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi* gxapi,
	gxapi::eFormat format,
	gxapi::RtvTexture2DArray desc) : ResourceViewBase(resource, handle) {
	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = format;
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_desc = RTVdesc;

	gxapi->CreateRenderTargetView(GetResource()._GetResourcePtr(), RTVdesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

gxapi::RenderTargetViewDesc RenderTargetView2D::GetDescription() const {
	return m_desc;
}


DepthStencilView2D::DepthStencilView2D(
	const Texture2D& resource,
	DSVHeap& heap,
	gxapi::eFormat format,
	gxapi::DsvTexture2DArray desc) : ResourceViewBase(resource, &heap) {
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = format;
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	heap.Create(GetResource(), DSVdesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

DepthStencilView2D::DepthStencilView2D(const Texture2D& resource, gxapi::DescriptorHandle handle, gxapi::IGraphicsApi* gxapi, gxapi::eFormat format, gxapi::DsvTexture2DArray desc)
	: ResourceViewBase(resource, handle) {
	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = format;
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_desc = DSVdesc;

	gxapi->CreateDepthStencilView(GetResource()._GetResourcePtr(), DSVdesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}



gxapi::DepthStencilViewDesc DepthStencilView2D::GetDescription() const {
	return m_desc;
}


BufferView::BufferView(
	const LinearBuffer& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc) : ResourceViewBase(resource, &heap),
							 m_format(format),
							 m_srvDesc(desc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::BUFFER;
	fullSrvDesc.buffer = desc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
}

BufferView::BufferView(
	const LinearBuffer& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi* gxapi,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc) : ResourceViewBase(resource, handle),
							 m_format(format),
							 m_srvDesc(desc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::BUFFER;
	fullSrvDesc.buffer = desc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
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
	gxapi::SrvTexture1DArray desc) : ResourceViewBase(resource, &heap),
									 m_format(format),
									 m_srvDesc(desc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE1DARRAY;
	fullSrvDesc.tex1DArray = desc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

TextureView1D::TextureView1D(
	const Texture1D& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi* gxapi,
	gxapi::eFormat format,
	gxapi::SrvTexture1DArray desc) : ResourceViewBase(resource, handle),
									 m_format(format),
									 m_srvDesc(desc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE1DARRAY;
	fullSrvDesc.tex1DArray = desc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}


gxapi::eFormat TextureView1D::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture1DArray& TextureView1D::GetDescription() const {
	return m_srvDesc;
}


TextureView2D::TextureView2D(
	const Texture2D& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvTexture2DArray srvDesc) : ResourceViewBase(resource, &heap),
										m_format(format),
										m_srvDesc(srvDesc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE2DARRAY;
	fullSrvDesc.tex2DArray = srvDesc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, srvDesc));
}

TextureView2D::TextureView2D(
	const Texture2D& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi* gxapi,
	gxapi::eFormat format,
	gxapi::SrvTexture2DArray srvDesc) : ResourceViewBase(resource, handle),
										m_format(format),
										m_srvDesc(srvDesc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE2DARRAY;
	fullSrvDesc.tex2DArray = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, srvDesc));
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
	gxapi::SrvTexture3D srvDesc) : ResourceViewBase(resource, &heap),
								   m_format(format),
								   m_srvDesc(srvDesc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE3D;
	fullSrvDesc.tex3D = srvDesc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, srvDesc));
}

TextureView3D::TextureView3D(
	const Texture3D& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi* gxapi,
	gxapi::eFormat format,
	gxapi::SrvTexture3D srvDesc) : ResourceViewBase(resource, handle),
								   m_format(format),
								   m_srvDesc(srvDesc) {
	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURE3D;
	fullSrvDesc.tex3D = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, srvDesc));
}


gxapi::eFormat TextureView3D::GetFormat() {
	return m_format;
}


const gxapi::SrvTexture3D& TextureView3D::GetDescription() const {
	return m_srvDesc;
}

TextureViewCube::TextureViewCube(
	const Texture2D& resource,
	CbvSrvUavHeap& heap,
	gxapi::eFormat format,
	gxapi::SrvTextureCubeArray srvDesc) : ResourceViewBase(resource, &heap),
										  m_format(format),
										  m_srvDesc(srvDesc) {
	if (resource.GetArrayCount() != 6 * srvDesc.numCubes) {
		throw InvalidArgumentException("Input texture must have array dimension 6*numCubes.");
	}

	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURECUBEARRAY;
	fullSrvDesc.texCubeArray = srvDesc;

	heap.CreateSRV(GetResource(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, srvDesc));
}

TextureViewCube::TextureViewCube(
	const Texture2D& resource,
	gxapi::DescriptorHandle handle,
	gxapi::IGraphicsApi* gxapi,
	gxapi::eFormat format,
	gxapi::SrvTextureCubeArray srvDesc) : ResourceViewBase(resource, handle),
										  m_format(format),
										  m_srvDesc(srvDesc) {
	if (resource.GetArrayCount() != 6 * srvDesc.numCubes) {
		throw InvalidArgumentException("Input texture must have array dimension 6*numCubes.");
	}

	gxapi::ShaderResourceViewDesc fullSrvDesc;
	fullSrvDesc.format = format;
	fullSrvDesc.dimension = gxapi::eSrvDimension::TEXTURECUBEARRAY;
	fullSrvDesc.texCubeArray = srvDesc;

	gxapi->CreateShaderResourceView(GetResource()._GetResourcePtr(), fullSrvDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, srvDesc));
}


gxapi::eFormat TextureViewCube::GetFormat() {
	return m_format;
}


const gxapi::SrvTextureCubeArray& TextureViewCube::GetDescription() const {
	return m_srvDesc;
}



//------------------------------------------------------------------------------
// RWs
//------------------------------------------------------------------------------


RWBufferView::RWBufferView(const LinearBuffer& resource,
						   CbvSrvUavHeap& heap,
						   gxapi::eFormat format,
						   gxapi::UavBuffer desc)
	: ResourceViewBase(resource, &heap),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::BUFFER;
	fullUavDesc.buffer = desc;

	heap.CreateUAV(GetResource(), fullUavDesc, GetHandle());
}

RWBufferView::RWBufferView(const LinearBuffer& resource,
						   gxapi::DescriptorHandle handle,
						   gxapi::IGraphicsApi* gxapi, gxapi::eFormat format,
						   gxapi::UavBuffer desc)
	: ResourceViewBase(resource, handle),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::BUFFER;
	fullUavDesc.buffer = desc;

	gxapi->CreateUnorderedAccessView(GetResource()._GetResourcePtr(), fullUavDesc, GetHandle());

	SetSubresourceList({ gxapi::ALL_SUBRESOURCES });
}

gxapi::eFormat RWBufferView::GetFormat() {
	return m_format;
}

const gxapi::UavBuffer& RWBufferView::GetDescription() const {
	return m_uavDesc;
}



RWTextureView1D::RWTextureView1D(const Texture1D& resource,
								 CbvSrvUavHeap& heap,
								 gxapi::eFormat format,
								 gxapi::UavTexture1DArray desc)
	: ResourceViewBase(resource, &heap),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::TEXTURE1D;
	fullUavDesc.tex1DArray = desc;

	heap.CreateUAV(GetResource(), fullUavDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

RWTextureView1D::RWTextureView1D(const Texture1D& resource,
								 gxapi::DescriptorHandle handle,
								 gxapi::IGraphicsApi* gxapi,
								 gxapi::eFormat format,
								 gxapi::UavTexture1DArray desc)
	: ResourceViewBase(resource, handle),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::TEXTURE1D;
	fullUavDesc.tex1DArray = desc;

	gxapi->CreateUnorderedAccessView(GetResource()._GetResourcePtr(), fullUavDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

gxapi::eFormat RWTextureView1D::GetFormat() {
	return m_format;
}

const gxapi::UavTexture1DArray& RWTextureView1D::GetDescription() const {
	return m_uavDesc;
}



RWTextureView2D::RWTextureView2D(const Texture2D& resource,
								 CbvSrvUavHeap& heap,
								 gxapi::eFormat format,
								 gxapi::UavTexture2DArray desc)
	: ResourceViewBase(resource, &heap),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::TEXTURE2DARRAY;
	fullUavDesc.tex2DArray = desc;

	heap.CreateUAV(GetResource(), fullUavDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

RWTextureView2D::RWTextureView2D(const Texture2D& resource,
								 gxapi::DescriptorHandle handle,
								 gxapi::IGraphicsApi* gxapi,
								 gxapi::eFormat format,
								 gxapi::UavTexture2DArray desc)
	: ResourceViewBase(resource, handle),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::TEXTURE2DARRAY;
	fullUavDesc.tex2DArray = desc;

	gxapi->CreateUnorderedAccessView(GetResource()._GetResourcePtr(), fullUavDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

gxapi::eFormat RWTextureView2D::GetFormat() {
	return m_format;
}

const gxapi::UavTexture2DArray& RWTextureView2D::GetDescription() const {
	return m_uavDesc;
}



RWTextureView3D::RWTextureView3D(const Texture3D& resource,
								 CbvSrvUavHeap& heap,
								 gxapi::eFormat format,
								 gxapi::UavTexture3D desc)
	: ResourceViewBase(resource, &heap),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::TEXTURE3D;
	fullUavDesc.tex3D = desc;

	heap.CreateUAV(GetResource(), fullUavDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

RWTextureView3D::RWTextureView3D(const Texture3D& resource,
								 gxapi::DescriptorHandle handle,
								 gxapi::IGraphicsApi* gxapi,
								 gxapi::eFormat format,
								 gxapi::UavTexture3D desc)
	: ResourceViewBase(resource, handle),
	  m_format(format),
	  m_uavDesc(desc) {
	gxapi::UnorderedAccessViewDesc fullUavDesc;
	fullUavDesc.format = format;
	fullUavDesc.dimension = gxapi::eUavDimension::TEXTURE3D;
	fullUavDesc.tex3D = desc;

	gxapi->CreateUnorderedAccessView(GetResource()._GetResourcePtr(), fullUavDesc, GetHandle());

	SetSubresourceList(CalcSubresourceList(resource, desc));
}

gxapi::eFormat RWTextureView3D::GetFormat() {
	return m_format;
}

const gxapi::UavTexture3D& RWTextureView3D::GetDescription() const {
	return m_uavDesc;
}



static std::vector<uint32_t> CalcSubresourceList(const Texture2D& obj, const gxapi::RtvTexture2DArray& desc) {
	std::vector<uint32_t> ret;
	for (uint32_t arr = desc.firstArrayElement; arr < desc.firstArrayElement + desc.activeArraySize; ++arr) {
		ret.push_back(obj.GetSubresourceIndex(desc.firstMipLevel, arr, desc.planeIndex));
	}
	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture2D& obj, const gxapi::DsvTexture2DArray& desc) {
	std::vector<uint32_t> ret;
	for (uint32_t arr = desc.firstArrayElement; arr < desc.firstArrayElement + desc.activeArraySize; ++arr) {
		ret.push_back(obj.GetSubresourceIndex(desc.firstMipLevel, arr, 0));
	}
	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture1D& obj, const gxapi::SrvTexture1DArray& desc) {
	std::vector<uint32_t> ret;

	uint32_t lastMip = desc.mostDetailedMip + desc.numMipLevels;
	if (desc.numMipLevels == 0 || desc.numMipLevels > obj.GetNumMiplevels()) {
		lastMip = obj.GetNumMiplevels();
	}

	for (uint32_t arr = desc.firstArrayElement; arr < desc.firstArrayElement + desc.activeArraySize; ++arr) {
		for (uint32_t mip = desc.mostDetailedMip; mip < lastMip; ++mip) {
			ret.push_back(obj.GetSubresourceIndex(mip, arr, 0));
		}
	}

	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture2D& obj, const gxapi::SrvTexture2DArray& desc) {
	std::vector<uint32_t> ret;

	uint32_t lastMip = desc.mostDetailedMip + desc.numMipLevels;
	if (desc.numMipLevels == 0 || desc.numMipLevels > obj.GetNumMiplevels()) {
		lastMip = obj.GetNumMiplevels();
	}

	for (uint32_t arr = desc.firstArrayElement; arr < desc.firstArrayElement + desc.activeArraySize; ++arr) {
		for (uint32_t mip = desc.mostDetailedMip; mip < lastMip; ++mip) {
			ret.push_back(obj.GetSubresourceIndex(mip, arr, 0));
		}
	}

	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture3D& obj, const gxapi::SrvTexture3D& desc) {
	std::vector<uint32_t> ret;

	uint32_t lastMip = desc.mostDetailedMip + desc.numMipLevels;
	if (desc.numMipLevels == 0 || desc.numMipLevels > obj.GetNumMiplevels()) {
		lastMip = obj.GetNumMiplevels();
	}

	for (uint32_t mip = desc.mostDetailedMip; mip < lastMip; ++mip) {
		ret.push_back(obj.GetSubresourceIndex(mip, 0));
	}

	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture2D& obj, const gxapi::SrvTextureCubeArray& desc) {
	std::vector<uint32_t> ret;

	uint32_t lastMip = desc.mostDetailedMip + desc.numMipLevels;
	if (desc.numMipLevels == 0 || desc.numMipLevels > obj.GetNumMiplevels()) {
		lastMip = obj.GetNumMiplevels();
	}

	uint32_t firstArray = desc.indexOfFirst2DTex;
	uint32_t lastArray = firstArray + 6 * desc.numCubes;

	for (uint32_t arr = firstArray; arr < lastArray; ++arr) {
		for (uint32_t mip = desc.mostDetailedMip; mip < lastMip; ++mip) {
			ret.push_back(obj.GetSubresourceIndex(mip, arr, 0));
		}
	}

	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture1D& obj, const gxapi::UavTexture1DArray& desc) {
	std::vector<uint32_t> ret;

	for (uint32_t arr = desc.firstArrayElement; arr < desc.firstArrayElement + desc.activeArraySize; ++arr) {
		ret.push_back(obj.GetSubresourceIndex(desc.mipLevel, arr, 0));
	}

	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture2D& obj, const gxapi::UavTexture2DArray& desc) {
	std::vector<uint32_t> ret;

	for (uint32_t arr = desc.firstArrayElement; arr < desc.firstArrayElement + desc.activeArraySize; ++arr) {
		ret.push_back(obj.GetSubresourceIndex(desc.mipLevel, arr, desc.planeIndex));
	}

	return ret;
}

static std::vector<uint32_t> CalcSubresourceList(const Texture3D& obj, const gxapi::UavTexture3D& desc) {
	std::vector<uint32_t> ret;

	ret.push_back(obj.GetSubresourceIndex(desc.mipLevel, 0));

	return ret;
}


} // namespace inl::gxeng
