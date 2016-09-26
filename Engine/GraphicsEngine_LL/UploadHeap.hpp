#pragma once

#include "GpuBuffer.hpp"

namespace inl {
namespace gxeng {


class UploadHeap {
public:
	struct UploadDescription {
		UploadDescription(GenericResource&& source, GenericResource* pDestination) :
		source(std::move(source)), pDestination(pDestination) {}

		GenericResource source;
		GenericResource* pDestination;
	};

public:
	UploadHeap(gxapi::IGraphicsApi* graphicsApi);

	void UploadToResource(LinearBuffer& target, const void* data, size_t size);

	std::vector<UploadDescription>& _GetQueuedUploads();

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::vector<UploadDescription> m_uploadQueue;
};


} // namespace gxeng
} // namespace inl
