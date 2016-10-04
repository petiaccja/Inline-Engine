#pragma once

#include "GpuBuffer.hpp"

#include <utility>

namespace inl {
namespace gxeng {


class UploadHeap {
public:
	struct UploadDescription {
		UploadDescription(GenericResource&& source, std::weak_ptr<GenericResource> destination, size_t offsetDst) :
		source(std::move(source)), destination(destination) {}

		GenericResource source;
		std::weak_ptr<GenericResource> destination;
		size_t offsetDst;
	};

public:
	UploadHeap(gxapi::IGraphicsApi* graphicsApi);

	void UploadToResource(std::weak_ptr<LinearBuffer> target, size_t offset, const void* data, size_t size);

	const std::vector<UploadDescription>& _GetQueuedUploads();
	void _ClearQueuedUploads();
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::vector<UploadDescription> m_uploadQueue;
};


} // namespace gxeng
} // namespace inl
