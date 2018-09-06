#pragma once

#include <memory>

#include <GraphicsEngine/Resources/IImage.hpp>
#include "ImageBase.hpp"


namespace inl {
namespace gxeng {


class Image : public IImage, protected ImageBase {
public:
	Image(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap) : ImageBase(memoryManager, descriptorHeap) {}

	void SetLayout(uint64_t width, uint32_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass) override;
	void Update(uint64_t x, uint32_t y, uint64_t width, uint32_t height, int mipLevel, const void* pixels, const IPixelReader& reader, size_t bytesPerRow = 0) override;

	size_t GetWidth() const override { return ImageBase::GetWidth(); }
	size_t GetHeight() const override { return ImageBase::GetHeight(); }
	ePixelChannelType GetChannelType() const override { return ImageBase::GetChannelType(); }
	int GetChannelCount() const override { return ImageBase::GetChannelCount(); }
	ePixelClass GetPixelClass() const override { return ImageBase::GetPixelClass(); }

	const TextureView2D& GetSrv() const;

private:
	void CreateResourceView(const Texture2D& texture) override;

private:
	TextureView2D m_resourceView;
};



} // namespace gxeng
} // namespace inl
