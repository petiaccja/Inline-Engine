#include "BackBufferManager.hpp"

#include "MemoryObject.hpp"

namespace inl {
namespace gxeng {


BackBufferManager::BackBufferManager(gxapi::IGraphicsApi* graphicsApi, gxapi::ISwapChain* swapChain) :
	m_graphicsApi(graphicsApi),
	m_swapChain(swapChain)
{
	const unsigned numBuffers = swapChain->GetDesc().numBuffers;

	{
		gxapi::DescriptorHeapDesc heapDesc;
		heapDesc.isShaderVisible = false;
		heapDesc.numDescriptors = numBuffers;
		heapDesc.type = gxapi::eDescriptorHeapType::RTV;
		m_descriptorHeap.reset(m_graphicsApi->CreateDescriptorHeap(heapDesc));
	}

	gxapi::RenderTargetViewDesc desc;
	desc.dimension = gxapi::eRtvDimension::TEXTURE2D;
	desc.tex2D.firstMipLevel = 0;
	desc.tex2D.planeIndex = 0;

	m_backBuffers.reserve(numBuffers);
	for (unsigned i = 0; i < numBuffers; i++) {
		gxapi::DescriptorHandle descHandle = m_descriptorHeap->At(i);
		MemoryObjDesc bufferDesc = MemoryObjDesc(swapChain->GetBuffer(i));

		auto resourceDesc = bufferDesc.resource->GetDesc();
		assert(resourceDesc.textureDesc.depthOrArraySize == 1);
		desc.format = resourceDesc.textureDesc.format;

		m_graphicsApi->CreateRenderTargetView(bufferDesc.resource.get(), desc, descHandle);

		BackBuffer highLevelBuffer(DescriptorReference(descHandle, nullptr), desc, std::move(bufferDesc));
		m_backBuffers.push_back(std::move(highLevelBuffer));
	}
}


BackBuffer& BackBufferManager::GetBackBuffer(unsigned index) {
	return m_backBuffers.at(index);
}


} // namespace gxeng
} // namespace inl
