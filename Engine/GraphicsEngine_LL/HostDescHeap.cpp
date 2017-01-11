#include "HostDescHeap.hpp"

#include "MemoryObject.hpp"

#include <cassert>
#include <array>
#include <type_traits>


namespace inl {
namespace gxeng {



RTVHeap::RTVHeap(gxapi::IGraphicsApi * graphicsApi) :
	HostDescHeap(graphicsApi, 32)
{}


void RTVHeap::Create(MemoryObject & resource, gxapi::RenderTargetViewDesc desc, gxapi::DescriptorHandle destination) {
	m_graphicsApi->CreateRenderTargetView(resource._GetResourcePtr(), desc, destination);
}


DSVHeap::DSVHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, 16)
{}


void DSVHeap::Create(MemoryObject & resource, gxapi::DepthStencilViewDesc desc, gxapi::DescriptorHandle destination) {
	m_graphicsApi->CreateDepthStencilView(resource._GetResourcePtr(), desc, destination);
}


PersistentResViewHeap::PersistentResViewHeap(gxapi::IGraphicsApi* graphicsApi) :
	HostDescHeap(graphicsApi, 256)
{}


void PersistentResViewHeap::CreateCBV(gxapi::ConstantBufferViewDesc desc, gxapi::DescriptorHandle destination) {
	m_graphicsApi->CreateConstantBufferView(desc, destination);
}


void PersistentResViewHeap::CreateSRV(MemoryObject& resource, gxapi::ShaderResourceViewDesc desc, gxapi::DescriptorHandle destination) {
	m_graphicsApi->CreateShaderResourceView(resource._GetResourcePtr(), desc, destination);
}



} // namespace inl
} // namespace gxeng
