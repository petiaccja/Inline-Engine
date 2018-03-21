#include "ImageBase.hpp"

namespace inl {
namespace gxeng {


ImageBase::ImageBase(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap) {
	assert(memoryManager != nullptr);
	m_memoryManager = memoryManager;
	m_descriptorHeap = descriptorHeap;

	m_channelCount = 0;
}


ImageBase::~ImageBase() {

}


void ImageBase::SetLayout(uint64_t width, uint32_t height, ePixelChannelType channelType, unsigned channelCount, ePixelClass pixelClass, unsigned arraySize) {
	gxapi::eFormat format;
	int resultChCnt = 0;
	if (!ConvertFormat(channelType, channelCount, pixelClass, format, resultChCnt)) {
		throw InvalidArgumentException("Unsupported texture format.");
	}


	Texture2DDesc resdesc(width, height, format, 0, arraySize);
	Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeap::CRITICAL, resdesc);

	// In case this throws an exception changes will be unrolled.
	CreateResourceView(texture);

	m_resource = std::move(texture);
	m_channelCount = channelCount;
	m_channelType = channelType;
	m_pixelClass = pixelClass;
}


void ImageBase::Update(uint64_t x, uint32_t y, uint64_t width, uint32_t height, unsigned mipLevel, unsigned arrayIndex, const void* pixels, const IPixelReader& reader, size_t bytesPerRow) {
	if (!m_resource) {
		throw InvalidStateException("Must create image first.");
	}

	if (x + width > GetWidth() || y + height > GetHeight()) {
		throw OutOfRangeException("Destination region out of bounds.");
	}

	if (GetChannelCount() != 4 && reader.GetChannelCount() == 3) {
		if (reader.GetChannelCount() != GetChannelCount()
			|| reader.GetPixelClass() != GetPixelClass()
			|| reader.GetChannelType() != GetChannelType())
		{
			throw NotImplementedException("Pixel types mismatch, conversion is not supported yet (but will be).");
		}
	}

	// Convert 3 channel pixels to 4 channels.
	std::unique_ptr<uint8_t> pixels4;
	if (reader.GetChannelCount() == 3) {
		size_t structureSize = reader.StructureSize();
		size_t structureSize4 = structureSize * 4 / 3;
		size_t channelSize = structureSize / 3;
		pixels4.reset(new uint8_t[structureSize4 * width * height]);
		size_t dstPitch = width * structureSize4;
		size_t srcPitch = bytesPerRow > 0 ? bytesPerRow : width * structureSize;
		for (size_t y = 0; y < height; ++y) {
			for (size_t x = 0; x < width; ++x) {
				uint8_t* dst = pixels4.get() + (y * dstPitch + x * structureSize4);
				memcpy(dst, (uint8_t*)pixels + (y * srcPitch + x * structureSize), structureSize);
				memset(dst + structureSize, 0, channelSize);
			}
		}
		pixels = pixels4.get();
	}

	// Upload data to gpu.
	m_memoryManager->GetUploadManager().Upload(
		m_resource,
		(uint32_t)x,
		(uint32_t)y,
		m_resource.GetSubresourceIndex(mipLevel, arrayIndex, 0),
		pixels,
		width,
		(uint32_t)height,
		m_resource.GetFormat(),
		bytesPerRow);
}


size_t ImageBase::GetWidth() const {
	if (m_resource) {
		return m_resource.GetWidth();
	}
	else {
		return 0;
	}
}


size_t ImageBase::GetHeight() const {
	if (m_resource) {
		return m_resource.GetHeight();
	}
	else {
		return 0;
	}
}


ePixelChannelType ImageBase::GetChannelType() const {
	return m_channelType;
}


int ImageBase::GetChannelCount() const {
	return m_channelCount;
}


ePixelClass ImageBase::GetPixelClass() const {
	return m_pixelClass;
}


bool ImageBase::ConvertFormat(ePixelChannelType channelType, int channelCount, ePixelClass pixelClass, gxapi::eFormat& fmt, int& resultingChannelCount) {
	using gxapi::eFormat;

	switch (channelType) {
		case ePixelChannelType::INT8_NORM:
		{
			eFormat arr[] = { eFormat::R8_UNORM, eFormat::R8G8_UNORM, eFormat::R8G8B8A8_UNORM , eFormat::R8G8B8A8_UNORM };
			fmt = arr[channelCount - 1];
			resultingChannelCount = channelCount == 3 ? 4 : channelCount;
			return true;
		}
		case ePixelChannelType::INT16_NORM:
		{
			eFormat arr[] = { eFormat::R16_UNORM, eFormat::R16G16_UNORM, eFormat::R16G16B16A16_UNORM , eFormat::R16G16B16A16_UNORM };
			fmt = arr[channelCount - 1];
			resultingChannelCount = channelCount == 3 ? 4 : channelCount;
			return true;
		}
		case ePixelChannelType::INT32:
		{
			eFormat arr[] = { eFormat::R32_UINT, eFormat::R32G32_UINT, eFormat::R32G32B32_UINT , eFormat::R32G32B32A32_UINT };
			fmt = arr[channelCount - 1];
			resultingChannelCount = channelCount;
			return true;
		}
		case ePixelChannelType::FLOAT32:
		{
			eFormat arr[] = { eFormat::R32_FLOAT, eFormat::R32G32_FLOAT, eFormat::R32G32B32_FLOAT, eFormat::R32G32B32A32_FLOAT };
			fmt = arr[channelCount - 1];
			resultingChannelCount = channelCount;
			return true;
		}
	}

	return false;
}



} // namespace gxeng
} // namespace inl