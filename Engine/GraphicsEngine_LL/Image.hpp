#pragma once

#include <memory>
//#include "MemoryObject.hpp"
//#include "Pixel.hpp"
//#include "MemoryManager.hpp"
//#include "ResourceView.hpp"

#include "ImageBase.hpp"


namespace inl {
namespace gxeng {


class Image : public ImageBase {
public:
	Image(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap) : ImageBase(memoryManager, descriptorHeap) {}

	void SetLayout(size_t width, size_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass);
	void Update(size_t x, size_t y, size_t width, size_t height, int mipLevel, const void* pixels, const IPixelReader& reader, size_t bytesPerRow = 0);

	const TextureView2D& GetSrv();

	void CreateResourceView(const Texture2D& texture) override;
private:
	TextureView2D m_resourceView;
};


/* OLD CODE, REMOVE IF NEW WORKS
class Image {
public:
	Image(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap);
	~Image();

	void SetLayout(size_t width, size_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass);
	void Update(size_t x, size_t y, size_t width, size_t height, const void* pixels, const IPixelReader& reader, size_t bytesPerRow = 0);

	size_t GetWidth();
	size_t GetHeight();
	ePixelChannelType GetChannelType() const;
	int GetChannelCount() const;
	ePixelClass GetPixelClass() const;

	std::shared_ptr<const TextureView2D> GetSrv();
protected:
	static bool Image::ConvertFormat(ePixelChannelType channelType, int channelCount, ePixelClass pixelClass, gxapi::eFormat& fmt, int& resultingChannelCount);
private:
	std::shared_ptr<TextureView2D> m_resource;
	ePixelChannelType m_channelType;
	int m_channelCount;
	ePixelClass m_pixelClass;
	MemoryManager* m_memoryManager;
	CbvSrvUavHeap* m_descriptorHeap;

};
*/


} // namespace gxeng
} // namespace inl
