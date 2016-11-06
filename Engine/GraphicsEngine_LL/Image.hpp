#pragma once

#include <memory>
#include "MemoryObject.hpp"
#include "Pixel.hpp"
#include "MemoryManager.hpp"


namespace inl {
namespace gxeng {



class Image {
public:
	Image(MemoryManager* memoryManager, PersistentResViewHeap* descriptorHeap);
	~Image();

	void SetLayout(size_t width, size_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass);
	void Update(size_t x, size_t y, size_t width, size_t height, void* pixels, const IPixelReader& reader);

	size_t GetWidth();
	size_t GetHeight();
	ePixelChannelType GetChannelType() const;
	int GetChannelCount() const;
	ePixelClass GetPixelClass() const;

	std::shared_ptr<const Texture2DSRV> GetSrv();
protected:
	static bool Image::ConvertFormat(ePixelChannelType channelType, int channelCount, ePixelClass pixelClass, gxapi::eFormat& fmt, int& resultingChannelCount);
private:
	std::shared_ptr<Texture2DSRV> m_resource;
	ePixelChannelType m_channelType;
	int m_channelCount;
	ePixelClass m_pixelClass;
	MemoryManager* m_memoryManager;
	PersistentResViewHeap* m_descriptorHeap;

};


} // namespace gxeng
} // namespace inl
