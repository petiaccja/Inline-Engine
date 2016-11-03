#include "UploadManager.hpp"

#include <GraphicsApi_LL/Common.hpp>

namespace inl {
namespace gxeng {


UploadManager::UploadManager(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi)
{
	std::lock_guard<std::mutex> lock(m_mtx);

	// Add a new queue before any frame starts to handle uploads at initialization.
	m_uploadQueues.push_back(std::vector<UploadDescription>()); 
}


void UploadManager::Upload(std::weak_ptr<LinearBuffer> target, size_t offset, const void* data, size_t size) {
	if (target.lock()->GetSize() < (offset+size)) {
		throw inl::gxapi::InvalidArgument("Target buffer is not large enough for the uploaded data to fit.", "target");
	}

	gxapi::IResource* uploadRes = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::UPLOAD),
		gxapi::eHeapFlags::NONE,
		gxapi::ResourceDesc::Buffer(size),
		//NOTE: GENERIC_READ is the required starting state for upload heap resources according to msdn
		// (also there is no need for resource state transition)
		gxapi::eResourceState::GENERIC_READ 
	);

	{
		std::lock_guard<std::mutex> lock(m_mtx);

		auto& currQueue = m_uploadQueues.back();

		MemoryObjectDescriptor desc;
		desc.resource = uploadRes;
		desc.resident = true;
		desc.deleter = std::default_delete<gxapi::IResource>();
		
		UploadDescription uploadDesc(
			MemoryObject(desc),
			target,
			offset
		);

		currQueue.push_back(std::move(uploadDesc));
		
		currQueue.back().source._SetResident(true);
	}

	gxapi::MemoryRange noReadRange{0, 0};
	void* stagePtr = uploadRes->Map(0, &noReadRange);
	memcpy(stagePtr, data, size);
	// Theres no need to unmap but leaving a resource mapped has a performance hit while debugging
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215(v=vs.85).aspx#mapping_and_unmapping
	uploadRes->Unmap(0, nullptr);
}


void UploadManager::Upload(std::weak_ptr<Texture2D> target,
						uint32_t offsetX,
						uint32_t offsetY,
						const void* data,
						size_t width,
						size_t height,
						gxapi::eFormat format
) {
	if (target.lock()->GetWidth() < (offsetX + width) || target.lock()->GetHeight() < (offsetY + height)) {
		throw inl::gxapi::InvalidArgument("Uploaded data does not fit inside target texture. (Uploaded size or offset is too large)", "target");
	}

	auto pixelSize = gxapi::GetFormatSizeInBytes(format);
	auto rowSize = width * pixelSize;
	size_t rowPitch = SnapUpwrads(rowSize, DUP_D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	auto requiredSize = rowPitch * height;

	gxapi::IResource* uploadRes = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::UPLOAD),
		gxapi::eHeapFlags::NONE,
		gxapi::ResourceDesc::Buffer(requiredSize),
		//NOTE: GENERIC_READ is the required starting state for upload heap resources according to msdn
		// (also there is no need for resource state transition)
		gxapi::eResourceState::GENERIC_READ 
	);

	{
		std::lock_guard<std::mutex> lock(m_mtx);

		auto& currQueue = m_uploadQueues.back();

		MemoryObjectDescriptor desc;
		desc.resource = uploadRes;
		desc.resident = true;
		desc.deleter = std::default_delete<gxapi::IResource>();

		UploadDescription uploadDesc(
			MemoryObject(desc),
			target,
			offsetX,
			offsetY,
			0,
			gxapi::TextureCopyDesc::Buffer(format, width, height, 1, 0)
		);

		currQueue.push_back(std::move(uploadDesc));

		currQueue.back().source._SetResident(true);
	}

	gxapi::MemoryRange noReadRange{0, 0};
	auto stagePtr = reinterpret_cast<uint8_t*>(uploadRes->Map(0, &noReadRange));
	auto byteData = reinterpret_cast<const uint8_t*>(data);
	//copy texture row-by-row
	for (size_t y = 0; y < height; y++) {
		memcpy(stagePtr + rowPitch*y, byteData + rowSize*y, rowSize);
	}
	uploadRes->Unmap(0, nullptr);
}


void UploadManager::OnFrameBeginDevice(uint64_t frameId) {
}


void UploadManager::OnFrameBeginHost(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mtx);

	m_uploadQueues.push_back(std::vector<UploadDescription>());
}


void UploadManager::OnFrameCompleteDevice(uint64_t frameId) {
	std::lock_guard<std::mutex> lock(m_mtx);

	m_uploadQueues.pop_front();
}


void UploadManager::OnFrameCompleteHost(uint64_t frameId) {
}


const std::vector<UploadManager::UploadDescription>& UploadManager::_GetQueuedUploads() {
	return m_uploadQueues.back();
}


size_t UploadManager::SnapUpwrads(size_t value, size_t gridSize) {
	// alignement should be power of two
	assert(((gridSize-1) & gridSize) == 0);
	return (value + (gridSize-1)) & ~(gridSize-1);
}


} // namespace gxeng
} // namespace inl
