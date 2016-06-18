#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

namespace inl {
namespace gxeng {


constexpr bool IS_ARRAY_SEPARATE = false;


//==================================
//Generic Resource

class GenericBuffer {
public:
	gxapi::IResource* GetResource();
	gxapi::IResource const* GetResource() const;

protected:
	void ResetResource(gxapi::IGraphicsApi * graphicsApi, gxapi::ResourceDesc desc);

private:
	std::unique_ptr<gxapi::IResource> m_resource;
};

//==================================


class VertexBuffer : public GenericBuffer {
public:
	VertexBuffer(){}
	VertexBuffer(gxapi::IGraphicsApi* graphicsApi, uint64_t size);
};


using IndexBuffer = VertexBuffer;


class TextureBase : public GenericBuffer {
public:
	uint64_t GetWidth() const;
	uint64_t GetHeight() const;
	uint64_t GetElementCount() const;
	uint64_t GetDepth() const;
	gxapi::eFormat GetFormat() const;
protected:
	TextureBase() {}
};


class Texture1D : public TextureBase {
public:
	Texture1D(){}
	Texture1D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format, uint16_t elementCount = 1);
};


class Texture2D : public TextureBase {
public:
	Texture2D(){}
	Texture2D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t elementCount = 1);
};


class Texture3D : public TextureBase {
public:
	Texture3D(){}
	Texture3D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format);
};


class TextureCubeMap : public TextureBase {
public:
	TextureCubeMap(){}
	TextureCubeMap(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format);
};


} // namespace gxeng
} // namespace inl
