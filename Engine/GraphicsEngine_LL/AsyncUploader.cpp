#include "AsyncUploader.hpp"

#include "../GraphicsApi_LL/IGraphicsCommandList.hpp"

#include <algorithm>

#if 0

namespace inl {
namespace gxeng {


using namespace gxapi;


AsyncUploader::AsyncUploader(gxapi::IGraphicsApi * graphicsApi) {
	m_graphicsApi = graphicsApi;
}


void AsyncUploader::FrameCompleted() {
	for (auto& curr : m_uploadResources) {
		curr.age += 1;
	}

	while (m_uploadResources.back().age >= CMDLIST_FINISH_FRAME_COUNT) {
		m_uploadResources.pop_back();
	}
}


void AsyncUploader::UploadBufferData(gxapi::IResource* dest, const void* src, size_t size) {
	std::lock_guard<std::shared_mutex> writerLock(m_mutex);

	{
		std::unique_ptr<gxapi::IResource> resource;
		resource.reset(m_graphicsApi->CreateCommittedResource(
			HeapProperties{eHeapType::UPLOAD},
			eHeapFlags::NONE,
			ResourceDesc::Buffer(size),
			eResourceState::GENERIC_READ
		));

		m_uploadResources.push_front(std::move(IntermediateUploadResource(dest, std::move(resource))));
	}
	auto& uploadRes = m_uploadResources.front();

	MemoryRange noReadRange = {0, 0};
	void* mapped = uploadRes.uploadResource->Map(0, &noReadRange);
	memcpy(mapped, src, size);
	uploadRes.uploadResource->Unmap(0, nullptr);


	static_assert(false, "Get a suitable list for copying");
	IGraphicsCommandList* list = nullptr;

	static_assert(false, "Add resource barrier transition on target");
	//list->CopyBuffer(uploadRes.target, 0, uploadRes.uploadResource.get(), 0, size);
}


bool AsyncUploader::IsFinished(gxapi::IResource const * dest) {
	std::shared_lock<std::shared_mutex> readerLock(m_mutex);
	auto end = m_uploadResources.end();
	return std::find_if(
		m_uploadResources.begin(),
		end,
		[dest](const IntermediateUploadResource& res){return res.target == dest;}
	) == end;
}


} // namespace gxeng
} // namespace inl

#endif
