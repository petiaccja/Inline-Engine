#include "UploadHeap.hpp"


namespace inl {
namespace gxeng {


UploadHeap::UploadHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{
	std::lock_guard<std::mutex> lock(m_mtx);

	// Add a new queue before any frame starts to handle uploads at initialization.
	m_uploadQueues.push_back(std::vector<UploadDescription>()); 
}


void UploadHeap::UploadToResource(std::weak_ptr<LinearBuffer> target, size_t offset, const void* data, size_t size) {
	if (target.lock()->GetSize() < (offset+size)) {
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
		std::lock_guard<std::mutex> lock(m_mtx);

		auto& currQueue = m_uploadQueues.back();
		
		UploadDescription uploadDesc(
			GenericResource(uploadRes),
			target,
			offset
		);

		currQueue.push_back(std::move(uploadDesc));
		
		currQueue.back().source._SetResident(true);
	}

	gxapi::MemoryRange noReadRange{0, 0};
	void* stagePtr = uploadRes->Map(0, &noReadRange);
	memcpy(stagePtr, data, size);
	// Theres no need to unmap but leaving a resource mapped has a performance hit while debugging
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215(v=vs.85).aspx#mapping_and_unmapping
	uploadRes->Unmap(0, nullptr);
}


void UploadHeap::OnFrameBeginDevice(uint64_t frameId) {
}


void UploadHeap::OnFrameBeginHost(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mtx);

	m_uploadQueues.push_back(std::vector<UploadDescription>());
}


void UploadHeap::OnFrameCompleteDevice(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mtx);

	m_uploadQueues.pop_front();
}


void UploadHeap::OnFrameCompleteHost(uint64_t frameId) {
}


const std::vector<UploadHeap::UploadDescription>& UploadHeap::_GetQueuedUploads() {
	return m_uploadQueues.back();
}


} // namespace gxeng
} // namespace inl
