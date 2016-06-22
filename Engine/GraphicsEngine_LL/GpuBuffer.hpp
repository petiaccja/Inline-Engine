#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

namespace inl {
namespace gxeng {



//==================================
// Generic Resource

class GenericResource {
public:
	void* GetVirtualAddress() const;
	gxapi::ResourceDesc GetDescriptor() const;



	//Dont use this function unless you really know what you are doing!
	gxapi::IResource* GetResource();

	//Dont use this function unless you really know what you are doing!
	gxapi::IResource const* GetResource() const;

protected:
	void ResetResource(gxapi::IGraphicsApi * graphicsApi, gxapi::ResourceDesc desc);

private:
	std::unique_ptr<gxapi::IResource> m_resource;
};

//==================================


//==================================
// Vertex buffer, index buffer

class VertexBuffer : public GenericResource {
public:
	VertexBuffer(){}
	VertexBuffer(gxapi::IGraphicsApi* graphicsApi, uint64_t size);

	uint64_t GetSize() const;
};


using IndexBuffer = VertexBuffer;

//==================================


//==================================
// Shared Texture Properties

class GenericTextureBase : virtual public GenericResource {
public:
	gxapi::eFormat GetFormat() const;
protected:
	GenericTextureBase() = default;
};


class ArrayTextureBase : virtual public GenericResource {
public:
	uint16_t GetElementCount() const;
protected:
	ArrayTextureBase() = default;
};


class HorizontalTextureBase : virtual public GenericResource {
public:
	uint64_t GetWidth() const;
protected:
	HorizontalTextureBase() = default;
};


class VerticalTextureBase : virtual public GenericResource {
public:
	uint64_t GetHeight() const;
protected:
	VerticalTextureBase() = default;
};

//==================================


//==================================
// Textures

class Texture1D :
	public GenericTextureBase,
	public ArrayTextureBase,
	public HorizontalTextureBase
{
public:
	Texture1D(){}
	Texture1D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format, uint16_t elementCount = 1);
};


class Texture2D :
	public GenericTextureBase,
	public ArrayTextureBase,
	public HorizontalTextureBase,
	public VerticalTextureBase
{
public:
	Texture2D(){}
	Texture2D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t elementCount = 1);
};


class Texture3D :
	public GenericTextureBase,
	public HorizontalTextureBase,
	public VerticalTextureBase
{
public:
	Texture3D(){}
	Texture3D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format);

	uint16_t GetDepth() const;
};


class TextureCubeMap :
	public GenericTextureBase,
	public HorizontalTextureBase,
	public VerticalTextureBase
{
public:
	TextureCubeMap(){}
	TextureCubeMap(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format);
};

//==================================

} // namespace gxeng
} // namespace inl
