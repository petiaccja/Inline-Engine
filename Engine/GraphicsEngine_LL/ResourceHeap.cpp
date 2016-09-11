#include "ResourceHeap.hpp"

#include "GpuBuffer.hpp"
#include "CopyCommandList.hpp"


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

		//NOTE: After the insertion has occured, no exception should leave uncatched
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


UploadHeap::UploadHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


void UploadHeap::UploadToResource(gxeng::CopyCommandList& cmdList, LinearBuffer& target, const void* data, size_t size) {
	if (target.GetSize() < size) {
		throw inl::gxapi::InvalidArgument("Target buffer is not large enough for the uploaded data to fit.", "target");
	}

	gxeng::DescriptorReference desc;
	desc.m_deleter = nullptr;
	desc.m_handle.cpuAddress = nullptr;
	desc.m_handle.gpuAddress = nullptr;

	m_stagedResources.push_back(GenericResource(std::move(desc)));
	GenericResource& staged = m_stagedResources.back();

	staged.m_deleter = nullptr;
	staged.m_resident = true;
	staged.m_resource = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::UPLOAD),
		gxapi::eHeapFlags::NONE,
		gxapi::ResourceDesc::Buffer(size),
		gxapi::eResourceState::COPY_DEST
	);

	gxapi::IResource* stagedRes = staged.m_resource;

	gxapi::MemoryRange noReadRange{0, 0};
	void* stagePtr = stagedRes->Map(0, &noReadRange);
	memcpy(stagePtr, data, size);
	// No need to unmap (see https://msdn.microsoft.com/en-us/library/windows/desktop/dn788712(v=vs.85).aspx)

	{
		gxapi::TransitionBarrier stageToSrc;
		stageToSrc.beforeState = gxapi::eResourceState::COPY_DEST;
		stageToSrc.afterState = gxapi::eResourceState::COPY_SOURCE;
		stageToSrc.resource = stagedRes;
		stageToSrc.subResource = 0xffffffff;
		stageToSrc.splitMode = gxapi::eResourceBarrierSplit::NORMAL;

		// Set stage's state to copy source
		cmdList.ResourceBarrier(stageToSrc);
		//Set target state to copy destination
		cmdList.RegisterResourceTransition(SubresourceID(target.m_resource, 0xffffffff), gxapi::eResourceState::COPY_DEST);
		//Copy
		cmdList.CopyBuffer(&target, 0, &staged, 0, size);
	}
}


} // namespace gxeng
} // namespace inl
