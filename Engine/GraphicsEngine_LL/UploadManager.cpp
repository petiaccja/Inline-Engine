#include "UploadManager.hpp"
#include "CopyCommandList.hpp"

#include <GraphicsApi_LL/Common.hpp>
#include <BaseLibrary/Exception/Exception.hpp>

#include <cassert>
#include <sstream>
#include <atomic>

namespace inl::gxeng {


UploadManager::UploadManager(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{
	std::lock_guard<std::mutex> lock(m_mtx);
}


void UploadManager::Upload(const LinearBuffer& target, size_t offset, const void* data, size_t size) {
	if (target.GetSize() < (offset + size)) {
		throw InvalidArgumentException("Target buffer is not large enough for the uploaded data to fit.", "target");
	}

	auto resource = CreateStagingResource(data, size);

	// Add upload to queue so that scheduler can enqueue it.
	std::lock_guard<std::mutex> lock(m_mtx);
	std::vector<UploadDescription>& currQueue = m_uploadFrames.back().uploads;

	UploadDescription uploadDesc(
		LinearBuffer(std::move(resource), true, eResourceHeap::UPLOAD),
		target,
		offset
	);
	currQueue.push_back(std::move(uploadDesc));
}


void UploadManager::Upload(const Texture2D& target,
						   uint32_t offsetX,
						   uint32_t offsetY,
						   uint32_t subresource,
						   const void* data,
						   uint64_t width,
						   uint32_t height,
						   gxapi::eFormat format,
						   size_t bytesPerRow)
{
	if (target.GetWidth() < (offsetX + width) || target.GetHeight() < (offsetY + height)) {
		throw InvalidArgumentException("Uploaded data does not fit inside target texture. (Uploaded size or offset is too large)", "target");
	}

	auto resource = CreateStagingResource(data, width, height, format, bytesPerRow);

	// Push upload description to queue so that the scheduler can enqueue the upload.
	std::lock_guard<std::mutex> lock(m_mtx);
	std::vector<UploadDescription>& currQueue = m_uploadFrames.back().uploads;

	UploadDescription uploadDesc(
		LinearBuffer(std::move(resource), true, eResourceHeap::UPLOAD),
		target,
		subresource,
		offsetX,
		offsetY,
		0,
		gxapi::TextureCopyDesc::Buffer(format, width, height, 1, 0)
	);

	currQueue.push_back(std::move(uploadDesc));
	currQueue.back().source._SetResident(true);

}

void UploadManager::UploadNow(CopyCommandList& commandList,
							  const LinearBuffer& target,
							  size_t offset,
							  const void* data,
							  size_t size)
{
	if (target.GetSize() < (offset + size)) {
		throw InvalidArgumentException("Target buffer is not large enough for the uploaded data to fit.", "target");
	}

	auto resource = CreateStagingResource(data, size);
	LinearBuffer source(std::move(resource), true, eResourceHeap::UPLOAD);

	// Enqueue resource copy.
	commandList.CopyBuffer(target, offset, source, 0, size);
}

void UploadManager::UploadNow(CopyCommandList& commandList,
							  const Texture2D& target,
							  uint32_t offsetX,
							  uint32_t offsetY,
							  uint32_t subresource,
							  const void* data,
							  uint64_t width,
							  uint32_t height,
							  gxapi::eFormat format,
							  size_t bytesPerRow)
{
	if (target.GetWidth() < (offsetX + width) || target.GetHeight() < (offsetY + height)) {
		throw InvalidArgumentException("Uploaded data does not fit inside target texture. (Uploaded size or offset is too large)", "target");
	}

	auto resource = CreateStagingResource(data, width, height, format, bytesPerRow);
	LinearBuffer source(std::move(resource), true, eResourceHeap::UPLOAD);

	// Enqueue resource copy.
	SubTexture2D dstPlace(subresource, Vector<intptr_t, 2>((intptr_t)offsetX, (intptr_t)offsetY));
	auto textureBufferDesc = gxapi::TextureCopyDesc::Buffer(format, width, height, 1, 0);
	commandList.CopyTexture(target, source, dstPlace, textureBufferDesc);
}


void UploadManager::OnFrameBeginDevice(uint64_t frameId) {
}


void UploadManager::OnFrameBeginHost(uint64_t frameId) {
}


void UploadManager::OnFrameCompleteDevice(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mtx);

	// Loop may be removed.
	int framesPopped = 0;
	while (!m_uploadFrames.empty() && m_uploadFrames.front().frameId <= frameId) {
		assert(m_uploadFrames.front().wasQueried);
		m_uploadFrames.pop_front();
		++framesPopped;
	}
	assert(framesPopped == 1);
}


void UploadManager::OnFrameCompleteHost(uint64_t frameId) {
}


void UploadManager::OnFrameBeginAwait(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mtx);

	UploadFrame uploadFrame;
	uploadFrame.frameId = frameId;
	m_uploadFrames.push_back(uploadFrame);
}


const std::vector<UploadManager::UploadDescription>& UploadManager::GetQueuedUploads() const {
	std::lock_guard<std::mutex> lock(m_mtx);

	assert(m_uploadFrames.size() > 0);
	m_uploadFrames.back().wasQueried = true;
	return m_uploadFrames.back().uploads;
}



size_t UploadManager::SnapUpwrads(size_t value, size_t gridSize) {
	// alignement should be power of two
	assert(((gridSize - 1) & gridSize) == 0);
	return (value + (gridSize - 1)) & ~(gridSize - 1);
}


MemoryObject::UniquePtr UploadManager::CreateStagingResource(const void* data, size_t size) {
	auto resource = MemoryObject::UniquePtr(
		m_graphicsApi->CreateCommittedResource(
			gxapi::HeapProperties(gxapi::eHeapType::UPLOAD),
			gxapi::eHeapFlags::NONE,
			gxapi::ResourceDesc::Buffer(size),
			//NOTE: GENERIC_READ is the required starting state for upload heap resources according to msdn
			// (also there is no need for resource state transition)
			gxapi::eResourceState::GENERIC_READ
		),
		std::default_delete<const gxapi::IResource>()
	);

	// Set resource name for tracking purposes.
	static std::atomic_uint64_t counter = 0;
	resource->SetName(("Buffer upload source" + std::to_string(counter++)).c_str());


	// Copy data to buffer.
	gxapi::MemoryRange noReadRange{ 0, 0 };
	void* stagePtr = resource->Map(0, &noReadRange);
	memcpy(stagePtr, data, size);
	// Theres no need to unmap but leaving a resource mapped has a performance hit while debugging
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215(v=vs.85).aspx#mapping_and_unmapping
	resource->Unmap(0, nullptr);

	return resource;
}


MemoryObject::UniquePtr UploadManager::CreateStagingResource(const void* data, uint64_t width, uint32_t height, gxapi::eFormat format, size_t bytesPerRow) {
	auto pixelSize = gxapi::GetFormatSizeInBytes(format);
	auto rowSize = width * pixelSize;
	size_t rowPitch = SnapUpwrads(rowSize, DUP_D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	auto requiredSize = bytesPerRow > 0 ? bytesPerRow : rowPitch * height;

	auto resource = MemoryObject::UniquePtr(
		m_graphicsApi->CreateCommittedResource(
			gxapi::HeapProperties(gxapi::eHeapType::UPLOAD),
			gxapi::eHeapFlags::NONE,
			gxapi::ResourceDesc::Buffer(requiredSize),
			// GENERIC_READ is the required starting state for upload heap resources according to MSDN.
			// (Also there is no need for resource state transition.)
			gxapi::eResourceState::GENERIC_READ
		),
		std::default_delete<const gxapi::IResource>()
	);

	// Set resource name for tracking purposes.
#ifdef _DEBUG
	static std::atomic_uint64_t counter = 0;
	resource->SetName(("Texture upload source" + std::to_string(counter++)).c_str());
#endif

	// Copy texture to upload buffer row-by-row.
	gxapi::MemoryRange noReadRange{ 0, 0 };
	auto stagePtr = reinterpret_cast<uint8_t*>(resource->Map(0, &noReadRange));
	auto byteData = reinterpret_cast<const uint8_t*>(data);
	for (size_t y = 0; y < height; y++) {
		memcpy(stagePtr + rowPitch*y, byteData + rowSize*y, rowSize);
	}
	resource->Unmap(0, nullptr);

	return resource;
}


} // namespace inl::gxeng
