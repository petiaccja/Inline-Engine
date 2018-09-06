#pragma once

#include <memory>
#include "MemoryObject.hpp"
#include <GraphicsEngine/Resources/Pixel.hpp>
#include "MemoryManager.hpp"
#include "ResourceView.hpp"


namespace inl::gxeng {



class ImageBase {
public:
	ImageBase(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap);
	ImageBase(const ImageBase&) = delete;
	ImageBase(ImageBase&&) = default;
	ImageBase& operator=(const ImageBase&) = delete;
	ImageBase& operator=(ImageBase&&) = default;
	~ImageBase();

	/// <summary> Returns the width of the image in pixels. </summary>
	size_t GetWidth() const;

	/// <summary> Returns the height of the image in pixels. </summary>
	size_t GetHeight() const;

	/// <summary> Return the numeric representation type of a pixel. See <see cref="ePixelChannelType"/>. </summary>
	ePixelChannelType GetChannelType() const;

	/// <summary> Returns the number of channels per pixel, i.e RGBA counts as 4. </summary>
	int GetChannelCount() const;

	/// <summary> Return the way pixels are interpreted. See <see cref="ePixelClass"/>. </summary>
	ePixelClass GetPixelClass() const;

protected:
	/// <summary> Allocates the underlying GPU-resident texture. </summary>
	/// <param name="width"> Width of the texture in pixels. </param>
	/// <param name="height"> Height of the texture in pixels. </param>
	/// <param name="channelType"> The numeric representation of a pixel channel. See <see cref="ePixelChannelType"/>. </param>
	/// <param name="channelCount"> Number of channels per pixel. </param>
	/// <param name="pixelClass"> How pixels are interpreted. See <see cref="ePixelClass"/>. </param>
	/// <param name="arraySize"> Specify 1 for simple images and 6 for cubemaps. </param>
	void SetLayout(uint64_t width, uint32_t height, ePixelChannelType channelType, unsigned channelCount, ePixelClass pixelClass, unsigned arraySize);

	/// <summary> Upload pixels as byte array to the GPU. </summary>
	/// <param name="x"> Where to insert the block of uploaded pixels. Top-left corner. </param>
	/// <param name="y"> Where to insert the block of uploaded pixels. Top-left corner. </param>
	/// <param name="width"> Width of the uploaded pixel block. </param>
	/// <param name="height"> Height of the uploaded pixel block. </param>
	/// <param name="mipLevel"> Target mip level of the texture. </param>
	/// <param name="arrayIdx"> Target array element of the texture. </param>
	/// <param name="pixels"> A pointer to the bytes representing the pixels. Use <see cref="Pixel"/> as helper. </param>
	/// <param name="reader"> Interprets byte stream. Implement <see cref="IPixelReader"/> or use <see cref="Pixel::Reader"/>. </param>
	/// <param name="bytesPerRow"> How many bytes to skip in <paramref name="pixels"/> for each row. Leave as 0 for no row padding. </param>
	/// <remarks> As you can't create multi-planed textures, uploading to specific plane is not supported. </remarks>
	void Update(uint64_t x, uint32_t y, uint64_t width, uint32_t height, unsigned mipLevel, unsigned arrayIdx, const void* pixels, const IPixelReader& reader, size_t bytesPerRow = 0);

	/// <summary> Converts simplified pixel format to GraphicsAPI format. </summary>
	static bool ConvertFormat(ePixelChannelType channelType, int channelCount, ePixelClass pixelClass, gxapi::eFormat& fmt, int& resultingChannelCount);
	
	/// <summary> This method is called whenever a new view needs to be created. </summary>
	/// <remarks> This must be implemented until the bottom-most subclass. </remarks>
	virtual void CreateResourceView(const Texture2D& texture) = 0;

protected:
	CbvSrvUavHeap* m_descriptorHeap;
private:
	Texture2D m_resource;
	ePixelChannelType m_channelType;
	int m_channelCount;
	ePixelClass m_pixelClass;
	MemoryManager* m_memoryManager;
};


} // namespace inl::gxeng