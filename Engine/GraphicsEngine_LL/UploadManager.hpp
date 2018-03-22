#pragma once

#include "PipelineEventListener.hpp"
#include "MemoryObject.hpp"

#include <utility>
#include <mutex>
#include <deque>
#include <list>

namespace inl::gxeng {

class CopyCommandList;


class UploadManager : public PipelineEventListener {
public:
	enum class DestType { BUFFER, TEXTURE_2D };
	struct UploadDescription {
		UploadDescription(LinearBuffer&& source,
						  const LinearBuffer& destination,
						  size_t bufferOffset) :
			source(std::move(source)),
			destination(destination),
			destType(DestType::BUFFER),
			dstOffsetX(bufferOffset) {}

		UploadDescription(LinearBuffer&& source,
						  const Texture2D& destination, unsigned dstSubresource,
						  size_t dstOffsetX, uint32_t dstOffsetY, uint32_t dstOffsetZ,
						  gxapi::TextureCopyDesc textureBufferDesc) :
			source(std::move(source)),
			destination(destination),
			dstSubresource(dstSubresource),
			destType(DestType::TEXTURE_2D),
			dstOffsetX(dstOffsetX), dstOffsetY(dstOffsetY), dstOffsetZ(dstOffsetZ),
			textureBufferDesc(textureBufferDesc) {}
		
		LinearBuffer source;

		// Destination is a weak pointer because it might get deleted before
		// the graphics engine starts to process the request.
		MemoryObject destination;
		DestType destType;

		size_t dstOffsetX; // also offset in linear buffer
		uint32_t dstOffsetY;
		uint32_t dstOffsetZ;
		unsigned dstSubresource;

		gxapi::TextureCopyDesc textureBufferDesc;
	};
private:
	struct UploadFrame {
		std::vector<UploadDescription> uploads;
		uint64_t frameId;
		mutable bool wasQueried = false; // Only for debugging. True if the scheduler asked for this batch.
	};

public:
	UploadManager(gxapi::IGraphicsApi* graphicsApi);

	/// <summary> Schedules uploading of data at the beginning of the next GPU frame. </summary>
	/// <param name="target"> Data is uploaded to this buffer. </param>
	/// <param name="data"> Pointer to the data to be uploaded. Can be deleted immediately after the call. </param>
	/// <param name="size"> Number of bytes pointed by <paramref name="data"/>. </param>
	/// <remarks> The next GPU frame is the one initiated by the next call to GraphicsEngine::Update. </remarks>
	void Upload(const LinearBuffer& target,
				size_t offset,
				const void* data,
				size_t size);

	// The pixels from the source image must be in row-major order inside memory.
	/// <summary> Schedules uploading of data at the beginning of the next GPU frame. </summary>
	/// <param name="target"> Data is uploaded to this texture. </param>
	/// <param name"offsetX"> Where to put new data in texture. </param>
	/// <param name"offsetY"> Where to put new data in texture. </param>
	/// <param name"subresource"> Which subresource of <paramref name="target"/>. </param>
	/// <param name"data"> Pointer to the row-major pixels to be uploaded. Can be deleted immediately after the call. </param>
	/// <param name"width"> Width of the subimage pointed by data. </param>
	/// <param name"height"> Height of the subimage pointed by data. </param>
	/// <param name"format"> Pixel format of subimage pointed by data. </param>
	/// <param name"bytesPerRow"> Row pitch of subimage pointed by data. </param>
	/// <remarks> The next GPU frame is the one initiated by the next call to GraphicsEngine::Update. </remarks>
	void Upload(const Texture2D& target,
				uint32_t offsetX,
				uint32_t offsetY,
				uint32_t subresource,
				const void* data,
				uint64_t width,
				uint32_t height,
				gxapi::eFormat format, 
				size_t bytesPerRow = 0);

	/// <summary> Schedules a data copy on the given command list immediately. </summary>
	/// <param name="commandList"> Data copy will be called on this command list. </param>
	/// <param name="target"> Data is uploaded to this buffer. </param>
	/// <param name="data"> Pointer to the data to be uploaded. Can be deleted immediately after the call. </param>
	/// <param name="size"> Number of bytes pointed by <paramref name="data"/>. </param>
	/// <remarks> The next GPU frame is the one initiated by the next call to GraphicsEngine::Update. </remarks>
	void UploadNow(CopyCommandList& commandList,
				   const LinearBuffer& target,
				   size_t offset,
				   const void* data,
				   size_t size);

	// The pixels from the source image must be in row-major order inside memory.
	/// <summary> Schedules a data copy on the given command list immediately. </summary>
	/// <param name="commandList"> Data copy will be called on this command list. </param>
	/// <param name="target"> Data is uploaded to this texture. </param>
	/// <param name"offsetX"> Where to put new data in texture. </param>
	/// <param name"offsetY"> Where to put new data in texture. </param>
	/// <param name"subresource"> Which subresource of <paramref name="target"/>. </param>
	/// <param name"data"> Pointer to the row-major pixels to be uploaded. Can be deleted immediately after the call. </param>
	/// <param name"width"> Width of the subimage pointed by data. </param>
	/// <param name"height"> Height of the subimage pointed by data. </param>
	/// <param name"format"> Pixel format of subimage pointed by data. </param>
	/// <param name"bytesPerRow"> Row pitch of subimage pointed by data. </param>
	/// <remarks> The next GPU frame is the one initiated by the next call to GraphicsEngine::Update. </remarks>
	void UploadNow(CopyCommandList& commandList,
				   const Texture2D& target,
				   uint32_t offsetX,
				   uint32_t offsetY,
				   uint32_t subresource,
				   const void* data,
				   uint64_t width,
				   uint32_t height,
				   gxapi::eFormat format,
				   size_t bytesPerRow = 0);

	void OnFrameBeginDevice(uint64_t frameId) override;
	void OnFrameBeginHost(uint64_t frameId) override;
	void OnFrameBeginAwait(uint64_t frameId) override;
	void OnFrameCompleteDevice(uint64_t frameId) override;
	void OnFrameCompleteHost(uint64_t frameId) override;

	/// <summary> Returns the batch of scheduled uploads for the upcoming frame. </summary>
	/// <remarks> If this function is called from the <see cref="Scheduler"/> - as it should be -
	///		the upcoming frame will be the one currently processed by the scheduler. </remarks>
	const std::vector<UploadDescription>& GetQueuedUploads() const;
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::list<UploadFrame> m_uploadFrames;

	mutable std::mutex m_mtx;

	// Creates and copies uploaded data into a staging GPU buffer (for buffers).
	MemoryObject::UniquePtr CreateStagingResource(const void* data, size_t size);
	// Creates and copies uploaded data into a staging GPU buffer (for textures).
	MemoryObject::UniquePtr CreateStagingResource(const void* data, uint64_t width, uint32_t height, gxapi::eFormat format, size_t bytesPerRow);
protected:
	static constexpr int DUP_D3D12_TEXTURE_DATA_PITCH_ALIGNMENT = 256;

private:
	static size_t SnapUpwrads(size_t value, size_t gridSize);
};


} // namespace inl::gxeng
