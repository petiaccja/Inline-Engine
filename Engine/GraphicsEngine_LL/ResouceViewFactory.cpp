#include "ResouceViewFactory.hpp"

#include "GpuBuffer.hpp"

namespace inl {
namespace gxeng {


ResouceViewFactory::ResouceViewFactory(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi),
	m_descHeap(graphicsApi)
{}


DepthStencilView ResouceViewFactory::CreateDepthStencilView(
	const std::shared_ptr<Texture2D>& resource,
	gxapi::DsvTexture2DArray desc
) {
	auto descRef = m_descHeap.AllocateOnTextureSpace();

	gxapi::DepthStencilViewDesc DSVdesc;
	DSVdesc.format = resource->GetFormat();
	DSVdesc.dimension = gxapi::eDsvDimension::TEXTURE2DARRAY;
	DSVdesc.tex2DArray = desc;

	gxapi::NotImplementedMethod("CreateDepthStencilView that takes three parameters is required to finish implementing this function");
	//m_graphicsApi->CreateDepthStencilView(resource->_GetResourcePtr(), RTVdesc, descRef.Get());

	return DepthStencilView(resource, std::move(descRef), DSVdesc);
}


RenderTargetView ResouceViewFactory::CreateRenderTargetView(
	const std::shared_ptr<Texture2D>& resource
) {
	auto descRef = m_descHeap.AllocateOnTextureSpace();

	gxapi::RenderTargetViewDesc desc;
	desc.format = resource->GetFormat();

	static_assert(
		std::is_same<std::remove_reference_t<decltype(resource)>::element_type, Texture2D>::value,
		"resource should be a Texture2D shared_ptr"
	);
	desc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	desc.tex2DArray.activeArraySize = 1;
	desc.tex2DArray.firstArrayElement = 0;
	desc.tex2DArray.planeIndex = 0;

	m_graphicsApi->CreateRenderTargetView(resource->_GetResourcePtr(), descRef.Get());

	return RenderTargetView(resource, std::move(descRef), desc);
}


RenderTargetView ResouceViewFactory::CreateRenderTargetView(
	const std::shared_ptr<Texture2D>& resource,
	gxapi::RtvTexture2DArray desc
) {
	auto descRef = m_descHeap.AllocateOnTextureSpace();

	gxapi::RenderTargetViewDesc RTVdesc;
	RTVdesc.format = resource->GetFormat();
	RTVdesc.dimension = gxapi::eRtvDimension::TEXTURE2DARRAY;
	RTVdesc.tex2DArray = desc;

	m_graphicsApi->CreateRenderTargetView(resource->_GetResourcePtr(), RTVdesc, descRef.Get());

	return RenderTargetView(resource, std::move(descRef), RTVdesc);
}


VertexBufferView ResouceViewFactory::CreateVertexBufferView(
	const std::shared_ptr<VertexBuffer>& resource,
	uint32_t stride,
	uint32_t size
) {
	return VertexBufferView(resource, stride, size);
}

BufferSRV ResouceViewFactory::CreateBufferSRV(
	const std::shared_ptr<LinearBuffer>& resource,
	gxapi::eFormat format,
	gxapi::SrvBuffer desc
) {
	auto descRef = m_descHeap.AllocateOnTextureSpace();

	gxapi::ShaderResourceViewDesc SRVdesc;
	SRVdesc.format = format;
	SRVdesc.dimension = gxapi::eSrvDimension::BUFFER;
	SRVdesc.buffer = desc;

	m_graphicsApi->CreateShaderResourceView(resource->_GetResourcePtr(), SRVdesc, descRef.Get());

	return BufferSRV(resource, std::move(descRef), format, desc);
}


} // namespace gxeng
} // namespace inl

