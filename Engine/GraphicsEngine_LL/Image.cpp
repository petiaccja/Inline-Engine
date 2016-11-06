#include "Image.hpp"

namespace inl {
namespace gxeng {


Image::Image(MemoryManager* memoryManager, PersistentResViewHeap* descriptorHeap) {
	assert(memoryManager != nullptr);
	m_memoryManager = memoryManager;
	m_descriptorHeap = descriptorHeap;

	m_channelCount = 0;
}


Image::~Image() {

}


void Image::SetLayout(size_t width, size_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass) {
	gxapi::eFormat format;
	int resultChCnt = 0;
	if (!ConvertFormat(channelType, channelCount, pixelClass, format, resultChCnt)) {
		throw std::invalid_argument("Unsupported texture format.");
	}
	if (channelCount != resultChCnt) {
		throw std::invalid_argument("3 channel not yet supported, use 4 channels instead.");
	}

	try {
		Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format);
		auto resourcePtr = std::make_shared<Texture2D>(std::move(texture));
		gxapi::SrvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.mipLevelClamping = 0;
		desc.mostDetailedMip = 0;
		desc.numMipLevels = -1;
		desc.planeIndex = 0;
		m_resource.reset(new Texture2DSRV(resourcePtr, *m_descriptorHeap, resourcePtr->GetFormat(), desc));

		m_channelCount = channelCount;
		m_channelType = channelType;
		m_pixelClass = pixelClass;
	}
	catch (...) {
		// might be able to do something useful
		throw;
	}
}

void Image::Update(size_t x, size_t y, size_t width, size_t height, void* pixels, const IPixelReader& reader) {
	if (!m_resource) {
		throw std::logic_error("Must create image first.");
	}

	if (x + width > GetWidth() || y + height > GetHeight()) {
		throw std::out_of_range("Destination region out of bounds.");
	}

	if (reader.GetChannelCount() != GetChannelCount() || reader.GetPixelClass() != GetPixelClass() || reader.GetChannelType() != GetChannelType()) {
		throw std::invalid_argument("Pixel types mismatch, conversion is not supported yet (but will be).");
	}

	m_memoryManager->GetUploadHeap().Upload(m_resource->GetResource(), x, y, pixels, width, height, m_resource->GetFormat());
}


size_t Image::GetWidth() {
	if (m_resource) {
		return m_resource->GetResource()->GetWidth();
	}
	else {
		return 0;
	}
}

size_t Image::GetHeight() {
	if (m_resource) {
		return m_resource->GetResource()->GetHeight();
	}
	else {
		return 0;
	}
}

ePixelChannelType Image::GetChannelType() const {
	return m_channelType;
}

int Image::GetChannelCount() const {
	return m_channelCount;
}

ePixelClass Image::GetPixelClass() const {
	return m_pixelClass;
}


std::shared_ptr<const Texture2DSRV> Image::GetSrv() {
	return m_resource;
}


bool Image::ConvertFormat(ePixelChannelType channelType, int channelCount, ePixelClass pixelClass, gxapi::eFormat& fmt, int& resultingChannelCount) {
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
}



} // namespace gxeng
} // namespace inl