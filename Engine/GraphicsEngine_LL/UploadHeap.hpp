#pragma once

#include "GpuBuffer.hpp"

#include <utility>

namespace inl {
namespace gxeng {


class UploadHeap {
public:
	struct UploadDescription {
		UploadDescription(GenericResource&& source, std::weak_ptr<GenericResource> destination) :
		source(std::move(source)), destination(destination) {}

		GenericResource source;
		std::weak_ptr<GenericResource> destination;
	};

public:
	UploadHeap(gxapi::IGraphicsApi* graphicsApi);

	void UploadToResource(std::weak_ptr<LinearBuffer> target, const void* data, size_t size);

	std::vector<UploadDescription>& _GetQueuedUploads();

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::vector<UploadDescription> m_uploadQueue;
};


} // namespace gxeng
} // namespace inl
