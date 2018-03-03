#pragma once

#include <memory>

#include "ImageBase.hpp"


namespace inl {
namespace gxeng {


class Image : public ImageBase {
public:
	Image(MemoryManager* memoryManager, CbvSrvUavHeap* descriptorHeap) : ImageBase(memoryManager, descriptorHeap) {}

	/// <summary> Allocates the underlying GPU-resident texture. </summary>
	/// <param name="width"> Width of the texture in pixels. </param>
	/// <param name="height"> Height of the texture in pixels. </param>
	/// <param name="channelType"> The numeric representation of a pixel channel. See <see cref="ePixelChannelType"/>. </param>
	/// <param name="channelCount"> Number of channels per pixel. </param>
	/// <param name="pixelClass"> How pixels are interpreted. See <see cref="ePixelClass"/>. </param>
	void SetLayout(uint64_t width, uint32_t height, ePixelChannelType channelType, int channelCount, ePixelClass pixelClass);

	/// <summary> Upload pixels as byte array to the GPU. </summary>
	/// <param name="x"> Where to insert the block of uploaded pixels. Top-left corner. </param>
	/// <param name="y"> Where to insert the block of uploaded pixels. Top-left corner. </param>
	/// <param name="width"> Width of the uploaded pixel block. </param>
	/// <param name="height"> Height of the uploaded pixel block. </param>
	/// <param name="mipLevel"> Target mip level of the texture. </param>
	/// <param name="pixels"> A pointer to the bytes representing the pixels. Use <see cref="Pixel"/> as helper. </param>
	/// <param name="reader"> Interprets byte stream. Implement <see cref="IPixelReader"/> or use <see cref="Pixel::Reader"/>. </param>
	/// <param name="bytesPerRow"> How many bytes to skip in <paramref name="pixels"/> for each row. Leave as 0 for no row padding. </param>
	void Update(uint64_t x, uint32_t y, uint64_t width, uint32_t height, int mipLevel, const void* pixels, const IPixelReader& reader, size_t bytesPerRow = 0);
	
	const TextureView2D& GetSrv() const;

private:
	void CreateResourceView(const Texture2D& texture) override;

private:
	TextureView2D m_resourceView;
};



} // namespace gxeng
} // namespace inl
