#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

namespace inl {
namespace gxeng {

//==================================
//Generic Resource

class GenericBuffer {
public:
	gxapi::IResource* GetResource();

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


class Texture1D : public GenericBuffer {
public:
	Texture1D(){}
	Texture1D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format);
};


class Texture1DArray : public GenericBuffer {
public:
	Texture1DArray(){}
	Texture1DArray(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format, uint16_t count);
};


class Texture2D : public GenericBuffer {
public:
	Texture2D(){}
	Texture2D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format);
};


class Texture2DArray : public GenericBuffer {
public:
	Texture2DArray(){}
	Texture2DArray(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t count);
};


class Texture3D : public GenericBuffer {
public:
	Texture3D(){}
	Texture3D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format);
};


class TextureCubeMap : public GenericBuffer {
public:
	TextureCubeMap(){}
	TextureCubeMap(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format);
};


} // namespace gxeng
} // namespace inl
