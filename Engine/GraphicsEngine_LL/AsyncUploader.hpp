#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include <shared_mutex>
#include <deque>

// TODO this class might not be needed.
#if 0

namespace inl {
namespace gxeng {


class AsyncUploader {
public:
	AsyncUploader(gxapi::IGraphicsApi* graphicsApi);

	void FrameCompleted();

	void UploadBufferData(gxapi::IResource* dest, const void* src, size_t size);
	bool IsFinished(gxapi::IResource const* dest);

protected:
	struct IntermediateUploadResource {
		IntermediateUploadResource(gxapi::IResource* target_, std::unique_ptr<gxapi::IResource>&& uploadResource_)
			: target(target_), uploadResource(std::move(uploadResource_)), age(0) {}

		gxapi::IResource* target;
		std::unique_ptr<gxapi::IResource> uploadResource;
		size_t age;
	};

	//TODO THIS IS A DUPLICATE COPIED FROM CONST BUFFER MANAGER
	// The number of frames needed to be completed for a
	// command list to get from being assembled to being finished
	// Expalnation: ("comp" means how many frames are completed)
	// Frames:      |   0 comp   |   1 comp  |   2 comp   |
	//         ...  |  assembly  |  process  |  finished  |  ...
	static constexpr uint8_t CMDLIST_FINISH_FRAME_COUNT = 2;

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	std::deque<IntermediateUploadResource> m_uploadResources;
	std::shared_mutex m_mutex;
};


} // namespace gxeng
} // namespace inl

#endif
