#include "ResouceViewFactory.hpp"

#include "GpuBuffer.hpp"

namespace inl {
namespace gxeng {


ResourceViewFactory::ResourceViewFactory(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi),
	m_CBV_SRV_UAV_Heap(graphicsApi, gxapi::eDesriptorHeapType::CBV_SRV_UAV),
	m_DSV_Heap(graphicsApi, gxapi::eDesriptorHeapType::DSV),
	m_RTV_Heap(graphicsApi, gxapi::eDesriptorHeapType::RTV)
{}


ConstBufferView ResourceViewFactory::CreateConstBufferView(
	const std::shared_ptr<PersistentConstBuffer>& resource
) {
	auto descRef = m_CBV_SRV_UAV_Heap.Allocate();

	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource->GetVirtualAddress();
	desc.sizeInBytes = resource->GetSize();

	m_graphicsApi->CreateConstantBufferView(desc, descRef.Get());

	return ConstBufferView(resource, std::move(descRef));
}


ConstBufferView ResourceViewFactory::CreateConstBufferView(
	const std::shared_ptr<VolatileConstBuffer>& resource,
	ScratchSpace* scratchSpace
) {
	auto descRef = scratchSpace->AllocateSingle();

	gxapi::ConstantBufferViewDesc desc;
	desc.gpuVirtualAddress = resource->GetVirtualAddress();
	desc.sizeInBytes = resource->GetSize();

	m_graphicsApi->CreateConstantBufferView(desc, descRef.Get());

	return ConstBufferView(resource, std::move(descRef));
}


DepthStencilView ResourceViewFactory::CreateDepthStencilView(
	const std::shared_ptr<Texture2D>& resource,
	gxapi::DsvTexture2DArray desc
) {
	auto descRef = m_DSV_Heap.Allocate();

	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = resource->GetFormat();
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	m_graphicsApi->CreateDepthStencilView(resource->_GetResourcePtr(), DSVdesc, descRef.Get());

	return DepthStencilView(resource, std::move(descRef), DSVdesc);
}


RenderTargetView ResourceViewFactory::CreateRenderTargetView(
	const std::shared_ptr<Texture2D>& resource
) {
	gxapi::RtvTexture2DArray desc;
	desc.activeArraySize = 1;
	desc.firstArrayElement = 0;
	desc.planeIndex = 0;
	desc.firstMipLevel = 0;

	return CreateRenderTargetView(resource, desc);
}


RenderTargetView ResourceViewFactory::CreateRenderTargetView(
	const std::shared_ptr<Texture2D>& resource,
	gxapi::RtvTexture2DArray desc
) {
	auto descRef = m_RTV_Heap.Allocate();

	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = resource->GetFormat();
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_graphicsApi->CreateRenderTargetView(resource->_GetResourcePtr(), RTVdesc, descRef.Get());

	return RenderTargetView(resource, std::move(descRef), RTVdesc);
}


VertexBufferView ResourceViewFactory::CreateVertexBufferView(
	const std::shared_ptr<VertexBuffer>& resource,
	uint32_t stride,
	uint32_t size
) {
	return VertexBufferView(resource, stride, size);
}


BufferSRV ResourceViewFactory::CreateBufferSRV(
	const std::shared_ptr<LinearBuffer>& resource,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc
) {
	auto descRef = m_CBV_SRV_UAV_Heap.Allocate();

	gxapi::ShaderResourceViewDesc SRVdesc;
	SRVdesc.format = format;
	SRVdesc.dimension = gxapi::eSrvDimension::BUFFER;
	SRVdesc.buffer = desc;

	m_graphicsApi->CreateShaderResourceView(resource->_GetResourcePtr(), SRVdesc, descRef.Get());

	return BufferSRV(resource, std::move(descRef), format, desc);
}


} // namespace gxeng
} // namespace inl

