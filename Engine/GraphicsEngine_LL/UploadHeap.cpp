#include "UploadHeap.hpp"

namespace inl {
namespace gxeng {


UploadHeap::UploadHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


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
		UploadDescription uploadDesc(
			GenericResource(uploadRes),
			target,
			offset
		);

		m_uploadQueue.push_back(std::move(uploadDesc));
	}
	m_uploadQueue.back().source._SetResident(true);

	gxapi::MemoryRange noReadRange{0, 0};
	void* stagePtr = uploadRes->Map(0, &noReadRange);
	memcpy(stagePtr, data, size);
	// Theres no need to unmap but leaving a resource mapped has a performance hit while debugging
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215(v=vs.85).aspx#mapping_and_unmapping
	uploadRes->Unmap(0, nullptr);
}


const std::vector<UploadHeap::UploadDescription>& UploadHeap::_GetQueuedUploads() {
	return m_uploadQueue;
}

void UploadHeap::_ClearQueuedUploads() {
#pragma message("Artúr ezt csináld meg, lécci! UploadHeap.cpp:57")
#pragma message("Becopyzom még vagy tízszer, hogy tuti lásd a sok buzi warning között :D")
#pragma message("Artúr ezt csináld meg, lécci! UploadHeap.cpp:57")
#pragma message("Artúr ezt csináld meg, lécci! UploadHeap.cpp:57")
#pragma message("Artúr ezt csináld meg, lécci! UploadHeap.cpp:57")
#pragma message("Artúr ezt csináld meg, lécci! UploadHeap.cpp:57")
#pragma message("Artúr ezt csináld meg, lécci! UploadHeap.cpp:57")
	throw std::runtime_error("Not implemented!");
	m_uploadQueue.clear();
}

} // namespace gxeng
} // namespace inl
