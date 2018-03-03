#include "Image.hpp"

namespace inl {
namespace gxeng {


void Image::SetLayout(uint64_t width, uint32_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass) {
	ImageBase::SetLayout(width, height, channelType, channelCount, pixelClass, 1);
}

void Image::Update(uint64_t x, uint32_t y, uint64_t width, uint32_t height, int mipLevel, const void* pixels, const IPixelReader& reader, size_t bytesPerRow) {
	ImageBase::Update(x, y, width, height, mipLevel, 0, pixels, reader, bytesPerRow);
}

const TextureView2D& Image::GetSrv() const {
	return m_resourceView;
}


void Image::CreateResourceView(const Texture2D& texture) {
	gxapi::SrvTexture2DArray srvdesc;
	srvdesc.activeArraySize = 1;
	srvdesc.firstArrayElement = 0;
	srvdesc.mipLevelClamping = 0;
	srvdesc.mostDetailedMip = 0;
	srvdesc.numMipLevels = 1; // change this back to -1
	srvdesc.planeIndex = 0;
	m_resourceView = TextureView2D(texture, *m_descriptorHeap, texture.GetFormat(), srvdesc);
}



} // namespace gxeng
} // namespace inl