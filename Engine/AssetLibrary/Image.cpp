#include "Image.hpp"
#include <cassert>

#include <BaseLibrary/Exception/Exception.hpp>


namespace inl {
namespace asset {


Image::Image(const std::string& file) {
	Load(file);
}


size_t Image::GetWidth() const {
	return m_image.isValid() ? m_image.getWidth() : 0;
}
size_t Image::GetHeight() const {
	return m_image.isValid() ? m_image.getHeight() : 0;
}

size_t Image::GetBytesPerRow() const {
	return m_image.isValid() ? m_image.getScanWidth() : 0;
}


eChannelType Image::GetType() const {
	eChannelType type;
	size_t count;
	TranslateImageType(type, count);
	return type;
}
int Image::GetChannelCount() const {
	eChannelType type;
	size_t count;
	TranslateImageType(type, count);
	return count;
}

void* Image::GetData() {
	return m_image.isValid() ? m_image.accessPixels() : nullptr;
}
const void* Image::GetData() const {
	return m_image.isValid() ? m_image.accessPixels() : nullptr;
}

template <eChannelType type, size_t count>
Pixel<type, count>& Image::At(size_t x, size_t y) {
	assert(x < GetWidth());
	assert(y < GetHeight());

	if (type != GetType() && count != GetChannelCount()) {
		throw InvalidCastException("The image does not contain this pixel type.");
	}

	return *(x + reinterpret_cast<Pixel<type, count>*>(m_image.getScanLine(y)));
}
template <eChannelType type, size_t count>
const Pixel<type, count>& Image::At(size_t x, size_t y) {
	assert(x < GetWidth());
	assert(y < GetHeight());

	if (type != GetType() && count != GetChannelCount()) {
		throw InvalidCastException("The image does not contain this pixel type.");
	}

	return *(x + reinterpret_cast<Pixel<type, count>*>(m_image.getScanLine(y)));
}

void Image::Create(size_t width, size_t height, eChannelType type, int channelCount) {
	FREE_IMAGE_TYPE fiType;
	int bpp;

	if (type == eChannelType::INT8 && channelCount <= 4) {
		fiType = FIT_BITMAP;
		bpp = 8 * channelCount;
	}
	else if (type == eChannelType::INT16) {
		switch (channelCount) {
			case 1: fiType = FIT_UINT16; break;
			case 3: fiType = FIT_RGB16; break;
			case 4: fiType = FIT_RGBA16; break;
			default: throw InvalidArgumentException("Unsupported pixel type.");
		}
	}
	else if (type == eChannelType::INT32) {
		switch (channelCount) {
			case 1: fiType = FIT_UINT32; break;
			default: throw InvalidArgumentException("Unsupported pixel type.");
		}
	}
	else if (type == eChannelType::FLOAT) {
		switch (channelCount) {
			case 1: fiType = FIT_FLOAT; break;
			case 3: fiType = FIT_RGBF; break;
			case 4: fiType = FIT_RGBAF; break;
			default: throw InvalidArgumentException("Unsupported pixel type.");
		}
	}
	else {
		throw InvalidArgumentException("Unsupported pixel type");
	}

	BOOL isImageOk = m_image.setSize(fiType, (unsigned)width, (unsigned)height, bpp, 0xFF00'0000, 0x00FF'0000, 0x0000'FF00);
	if (!isImageOk) {
		throw RuntimeException("Failed to create internal image.");
	}
}

void Image::Load(const std::string& file) {
	if (!m_image.load(file.c_str())) {
		throw RuntimeException("Failed to load image.");
	}
}


void Image::TranslateImageType(eChannelType& typeOut, size_t& countOut) const {
	FREE_IMAGE_TYPE type = m_image.getImageType();
	FREE_IMAGE_COLOR_TYPE colorType = m_image.getColorType();

	if (type == FIT_BITMAP) {
		int bitdepth = m_image.getBitsPerPixel();
		switch (bitdepth) {
			case 8: typeOut = eChannelType::INT8; countOut = 1; break;
			case 16: typeOut = eChannelType::INT8; countOut = 2; break;
			case 24: typeOut = eChannelType::INT8; countOut = 3; break;
			case 32: typeOut = eChannelType::INT8; countOut = 4; break;
			default: throw OutOfRangeException("No matching type.");
		}
	}
	else if (type == FIT_UINT16) {
		typeOut = eChannelType::INT16;
		countOut = 1;
	}
	else if (type == FIT_FLOAT) {
		typeOut = eChannelType::INT16;
		countOut = 1;
	}
	else if (type == FIT_UINT32) {
		typeOut = eChannelType::INT32;
		countOut = 1;
	}
	else if (type == FIT_RGB16) {
		typeOut = eChannelType::INT16;
		countOut = 3;
	}
	else if (type == FIT_RGBA16) {
		typeOut = eChannelType::INT16;
		countOut = 4;
	}
	else if (type == FIT_RGBF) {
		typeOut = eChannelType::FLOAT;
		countOut = 3;
	}
	else if (type == FIT_RGBAF) {
		typeOut = eChannelType::FLOAT;
		countOut = 4;
	}
	else {
		throw OutOfRangeException("No matching type.");
	}
}


}
}
