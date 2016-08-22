#include "ResourceHeap.hpp"

#include "GpuBuffer.hpp"


namespace inl {
namespace gxeng {
namespace impl {


CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi * graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


gxapi::IResource* CriticalBufferHeap::Allocate(GenericResource* owner, gxapi::ResourceDesc desc) {
	gxapi::IResource* retval;
	{
		std::unique_ptr<gxapi::IResource> allocation(
			m_graphicsApi->CreateCommittedResource(
				gxapi::HeapProperties(gxapi::eHeapType::DEFAULT),
				gxapi::eHeapFlags::NONE,
				desc,
				gxapi::eResourceState::COMMON
			)
		);

		retval = allocation.get();

		//After the insertion has occured, no exception should leave uncatched
		m_resources.insert({owner, std::move(allocation)});
	}

	return retval;
}


void CriticalBufferHeap::ReleaseUnderlying(GenericResource* owner) {
	m_resources.erase(owner);
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

	m_backBuffers.reserve(numBuffers);
	for (unsigned i = 0; i < numBuffers; i++) {
		gxapi::DescriptorHandle descHandle = m_descriptorHeap->At(i);
		gxapi::IResource* lowLeveBuffer = swapChain->GetBuffer(i);
		m_graphicsApi->CreateRenderTargetView(lowLeveBuffer, descHandle);

		DescriptorReference descRef;
		descRef.m_handle = descHandle;
		descRef.m_deleter = nullptr; // Descriptors needn't be freed until this heap exists.

		Texture2D highLevelBuffer(std::move(descRef));
		highLevelBuffer.m_resource = lowLeveBuffer;
		highLevelBuffer.m_deleter = nullptr; // Underlying resource deallocation is managed by the swap chain!
		highLevelBuffer.m_resident = true; // I guess...

		m_backBuffers.push_back(std::move(highLevelBuffer));
	}
}


Texture2D& BackBufferHeap::GetBackBuffer(unsigned index) {
	return m_backBuffers.at(index);
}

} // namespace gxeng
} // namespace inl
