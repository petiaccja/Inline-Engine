#pragma once

#include "PipelineEventListener.hpp"
#include "GpuBuffer.hpp"

#include <utility>
#include <mutex>
#include <deque>

namespace inl {
namespace gxeng {


class UploadHeap : public PipelineEventListener {
public:
	struct UploadDescription {
		UploadDescription(GenericResource&& source, std::weak_ptr<GenericResource> destination, size_t offsetDst) :
		source(std::move(source)), destination(destination), offsetDst(offsetDst) {}

		GenericResource source;
		std::weak_ptr<GenericResource> destination;
		size_t offsetDst;
	};

public:
	UploadHeap(gxapi::IGraphicsApi* graphicsApi);

	void UploadToResource(std::weak_ptr<LinearBuffer> target, size_t offset, const void* data, size_t size);

	void OnFrameBeginDevice(uint64_t frameId) override;
	void OnFrameBeginHost(uint64_t frameId) override;
	void OnFrameCompleteDevice(uint64_t frameId) override;
	void OnFrameCompleteHost(uint64_t frameId) override;

	const std::vector<UploadDescription>& _GetQueuedUploads();
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::deque<std::vector<UploadDescription>> m_uploadQueues;

	std::mutex m_mtx;
};


} // namespace gxeng
} // namespace inl
