#include "Cubemap.hpp"

namespace inl::gxeng {



void Cubemap::SetLayout(uint64_t width, uint32_t height, ePixelChannelType channelType, unsigned channelCount, ePixelClass pixelClass) {
	ImageBase::SetLayout(width, height, channelType, channelCount, pixelClass, 6);
}


void Cubemap::Update(uint64_t x, uint32_t y, uint64_t width, uint32_t height, unsigned mipLevel, Face face, const void* pixels, const IPixelReader& reader, size_t bytesPerRow) {
	ImageBase::Update(x, y, width, height, mipLevel, GetFaceIndex(face.facePosition), pixels, reader, bytesPerRow);
}


const TextureViewCube& Cubemap::GetSrv() {
	return m_resourceView;
}


void Cubemap::CreateResourceView(const Texture2D& texture) {
	assert(texture.GetArrayCount() == 6);

	gxapi::SrvTextureCubeArray srvdesc;
	srvdesc.indexOfFirst2DTex = 0;
	srvdesc.mipLevelClamping = 0;
	srvdesc.mostDetailedMip = 0;
	srvdesc.numCubes = 1;
	srvdesc.numMipLevels = -1;
	m_resourceView = TextureViewCube(texture, *m_descriptorHeap, texture.GetFormat(), srvdesc);
}


int Cubemap::GetFaceIndex(eAxis facePosition) {
	// I got this from MSDN, so it works for directx gxapi implementation.
	// I don't know about the rest though.
	switch (facePosition) {
		case inl::gxeng::Cubemap::POS_X:
			return 0;
		case inl::gxeng::Cubemap::POS_Y:
			return 2;
		case inl::gxeng::Cubemap::POS_Z:
			return 4;
		case inl::gxeng::Cubemap::NEG_X:
			return 1;
		case inl::gxeng::Cubemap::NEG_Y:
			return 3;
		case inl::gxeng::Cubemap::NEG_Z:
			return 5;
		default:
			assert(false);
			return -1;
	}
}



} // namespace inl::gxeng