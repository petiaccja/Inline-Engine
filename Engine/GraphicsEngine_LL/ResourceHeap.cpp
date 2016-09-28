#include "ResourceHeap.hpp"

#include "GpuBuffer.hpp"
#include "CopyCommandList.hpp"


namespace inl {
namespace gxeng {
namespace impl {



CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi * graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


InitialResourceParameters CriticalBufferHeap::Allocate(DescriptorReference&& viewRef, gxapi::ResourceDesc desc) {
	InitialResourceParameters result{std::move(viewRef)};

	result.resource = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::DEFAULT),
		gxapi::eHeapFlags::NONE,
		desc,
		gxapi::eResourceState::COMMON
	);

	result.residency = true;

	m_graphicsApi->CreateShaderResourceView(result.resource, result.desc.Get());

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

	m_backBuffers.reserve(numBuffers);
	for (unsigned i = 0; i < numBuffers; i++) {
		gxapi::DescriptorHandle descHandle = m_descriptorHeap->At(i);
		gxapi::IResource* lowLeveBuffer = swapChain->GetBuffer(i);
		m_graphicsApi->CreateRenderTargetView(lowLeveBuffer, descHandle);

		// Descriptors needn't be freed until this heap exists.
		DescriptorReference descRef(descHandle, nullptr);

		// Underlying resource deallocation is managed by the swap chain!
		Texture2D highLevelBuffer(std::move(descRef), lowLeveBuffer, [](gxapi::IResource*){});
		highLevelBuffer._SetResident(true); // I guess...

		m_backBuffers.push_back(std::move(highLevelBuffer));
	}
}


Texture2D& BackBufferHeap::GetBackBuffer(unsigned index) {
	return m_backBuffers.at(index);
}


UploadHeap::UploadHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


void UploadHeap::UploadToResource(LinearBuffer& target, const void* data, size_t size) {
	if (target.GetSize() < size) {
		throw inl::gxapi::InvalidArgument("Target buffer is not large enough for the uploaded data to fit.", "target");
	}

	gxapi::DescriptorHandle nullHandle;
	nullHandle.cpuAddress = nullptr;
	nullHandle.gpuAddress = nullptr;

	gxeng::DescriptorReference desc(nullHandle, nullptr);

	gxapi::IResource* uploadRes = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::UPLOAD),
		gxapi::eHeapFlags::NONE,
		gxapi::ResourceDesc::Buffer(size),
		//NOTE: GENERIC_READ is the required starting state for upload heap resources according to msdn
		// (also there is no need for resource state transition)
		gxapi::eResourceState::GENERIC_READ 
	);

	{
		UploadDescription uploadDesc(
			GenericResource(
				std::move(desc),
				uploadRes
			),
			&target
		);

		m_uploadQueue.push_back(std::move(uploadDesc));
	}
	m_uploadQueue.back().source._SetResident(true);

	gxapi::MemoryRange noReadRange{0, 0};
	void* stagePtr = uploadRes->Map(0, &noReadRange);
	memcpy(stagePtr, data, size);
	// No need to unmap (see https://msdn.microsoft.com/en-us/library/windows/desktop/dn788712(v=vs.85).aspx)
}


const std::vector<UploadHeap::UploadDescription>& UploadHeap::_GetQueuedUploads() {
	return m_uploadQueue;
}


} // namespace gxeng
} // namespace inl
