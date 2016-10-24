#include "ResourceHeap.hpp"

#include "GpuBuffer.hpp"
#include "CopyCommandList.hpp"

#include <iostream>


namespace inl {
namespace gxeng {
namespace impl {



CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi * graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


InitialResourceParameters CriticalBufferHeap::Allocate(gxapi::ResourceDesc desc, gxapi::ClearValue* clearValue) {
	InitialResourceParameters result;

	result.resource = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::DEFAULT),
		gxapi::eHeapFlags::NONE,
		desc,
		gxapi::eResourceState::COMMON,
		clearValue
	);

	result.residency = true;

	return result;
}


} // namespace impl


BackBufferHeap::BackBufferHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::ISwapChain* swapChain) :
	m_graphicsApi(graphicsApi),
	m_swapChain(swapChain)
{
	const unsigned numBuffers = swapChain->GetDesc().numBuffers;

	{
		gxapi::DescriptorHeapDesc heapDesc;
		heapDesc.isShaderVisible = false;
		heapDesc.numDescriptors = numBuffers;
		heapDesc.type = gxapi::eDesriptorHeapType::RTV;
		m_descriptorHeap.reset(m_graphicsApi->CreateDescriptorHeap(heapDesc));
	}

	gxapi::RenderTargetViewDesc desc;
	desc.dimension = gxapi::eRtvDimension::TEXTURE2D;
	desc.tex2D.firstMipLevel = 0;
	desc.tex2D.planeIndex = 0;

	m_backBuffers.reserve(numBuffers);
	for (unsigned i = 0; i < numBuffers; i++) {
		gxapi::DescriptorHandle descHandle = m_descriptorHeap->At(i);
		std::unique_ptr<gxapi::IResource> lowLeveBuffer(swapChain->GetBuffer(i));

		auto resourceDesc = lowLeveBuffer->GetDesc();
		assert(resourceDesc.textureDesc.depthOrArraySize == 1);
		desc.format = resourceDesc.textureDesc.format;

		m_graphicsApi->CreateRenderTargetView(lowLeveBuffer.get(), desc, descHandle);

		BackBuffer highLevelBuffer(DescriptorReference(descHandle, nullptr), desc, lowLeveBuffer.release());
		highLevelBuffer._SetResident(true); // I guess...

		m_backBuffers.push_back(std::move(highLevelBuffer));
	}
}


BackBuffer& BackBufferHeap::GetBackBuffer(unsigned index) {
	return m_backBuffers.at(index);
}



} // namespace gxeng
} // namespace inl
