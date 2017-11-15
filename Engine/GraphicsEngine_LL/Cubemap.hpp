#pragma once

#include <memory>

#include "ImageBase.hpp"


namespace inl::gxeng {


class Cubemap : public ImageBase {
public:
	enum eAxis {
		POS_X,
		POS_Y,
		POS_Z,
		NEG_X,
		NEG_Y,
		NEG_Z,
	};
	struct Face {
		eAxis facePosition; /// <summary> Where the updated cubemap face is in 3D space. </summary>
		//eAxis imageU; /// <summary> Which axis the uploaded image's horizontal axis corresponds to in 3D space. </summary>
		//eAxis imageV; /// <summary> Which axis the uploaded image's vertical axis corresponds to in 3D space. </summary>
	};

	Cubemap(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap) : ImageBase(memoryManager, descriptorHeap) {}

	void SetLayout(size_t width, size_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass);
	void Update(size_t x, size_t y, size_t width, size_t height, int mipLevel, Face face, const void* pixels, const IPixelReader& reader, size_t bytesPerRow = 0);

	const TextureViewCube& GetSrv();

	void CreateResourceView(const Texture2D& texture) override;

private:
	static int GetFaceIndex(eAxis facePosition);

private:
	TextureViewCube m_resourceView;
};



} // namespace inl::gxeng