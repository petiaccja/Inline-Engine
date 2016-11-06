#include "Image.hpp"

namespace inl {
namespace gxeng {


Image::Image(MemoryManager* memoryManager) {
	assert(memoryManager != nullptr);
	m_memoryManager = memoryManager;

	m_channelCount = 0;
}


Image::~Image() {

}


void Image::SetLayout(size_t width, size_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass) {
	gxapi::eFormat format = ConvertFormat(channelType, channelCount, pixelClass);

	try {
		Texture2D texture = m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, format);
		m_resource = std::make_shared<Texture2D>(std::move(texture));

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

	m_memoryManager->GetUploadHeap().Upload(m_resource, x, y, pixels, width, height, m_resource->GetFormat());
}


size_t Image::GetWidth() {
	if (m_resource) {
		return m_resource->GetWidth();
	}
	else {
		return 0;
	}
}

size_t Image::GetHeight() {
	if (m_resource) {
		return m_resource->GetHeight();
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


gxapi::eFormat Image::ConvertFormat(ePixelChannelType channelType, int channelCount, ePixelClass pixelClass) {
	assert(false);
	return gxapi::eFormat::R8G8B8A8_UNORM;
}



} // namespace gxeng
} // namespace inl