#pragma once

#include "PipelineEventListener.hpp"
#include "MemoryObject.hpp"

#include <utility>
#include <mutex>
#include <deque>

namespace inl {
namespace gxeng {


class UploadManager : public PipelineEventListener {
public:
	struct UploadDescription {
		UploadDescription(MemoryObject&& source,
						  const std::weak_ptr<MemoryObject>& destination,
						  size_t bufferOffset) :
			source(std::move(source)),
			destination(destination),
			dstOffsetX(bufferOffset) {}

		UploadDescription(MemoryObject&& source,
						  const std::weak_ptr<MemoryObject>& destination,
						  size_t dstOffsetX, uint32_t dstOffsetY, uint32_t dstOffsetZ,
						  gxapi::TextureCopyDesc textureBufferDesc) :
			source(std::move(source)),
			destination(destination),
			dstOffsetX(dstOffsetX), dstOffsetY(dstOffsetY), dstOffsetZ(dstOffsetZ),
			textureBufferDesc(textureBufferDesc) {}
		
		MemoryObject source;

		// Destination is a weak pointer because it might get deleted before
		// the graphics engine starts to process the request.
		std::weak_ptr<MemoryObject> destination;

		size_t dstOffsetX; // also offset in linear buffer
		uint32_t dstOffsetY;
		uint32_t dstOffsetZ;

		gxapi::TextureCopyDesc textureBufferDesc;
	};

public:
	UploadManager(gxapi::IGraphicsApi* graphicsApi);

	void Upload(std::weak_ptr<LinearBuffer> target, size_t offset, const void* data, size_t size);

	// The pixels from the source image must be in row-major order inside memory.
	void Upload(std::weak_ptr<Texture2D> target, uint32_t offsetX, uint32_t offsetY, const void* data, size_t width, size_t height, gxapi::eFormat format);

	void OnFrameBeginDevice(uint64_t frameId) override;
	void OnFrameBeginHost(uint64_t frameId) override;
	void OnFrameCompleteDevice(uint64_t frameId) override;
	void OnFrameCompleteHost(uint64_t frameId) override;

	const std::vector<UploadDescription>& _GetQueuedUploads();
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::deque<std::vector<UploadDescription>> m_uploadQueues;

	std::mutex m_mtx;

protected:
	static constexpr int DUP_D3D12_TEXTURE_DATA_PITCH_ALIGNMENT = 256;

private:
	static size_t SnapUpwrads(size_t value, size_t gridSize);
};


} // namespace gxeng
} // namespace inl
