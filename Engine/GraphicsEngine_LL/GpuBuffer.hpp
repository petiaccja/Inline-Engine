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
	gxapi::DescriptorHandle m_viewHandle; // dont forget it when copying the resource
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

class GenericTextureBase : public GenericResource {
public:
	uint64_t GetWidth() const;
	gxapi::eFormat GetFormat() const;
protected:
	GenericTextureBase() = default;
};

//==================================


//==================================
// Textures

class Texture1D : public GenericTextureBase {
public:
	Texture1D(){}
	Texture1D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, gxapi::eFormat format, uint16_t elementCount = 1);

	uint16_t GetArrayCount() const;
};


class Texture2D : public GenericTextureBase {
public:
	Texture2D(){}
	Texture2D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format, uint16_t elementCount = 1);

	uint64_t GetHeight() const;
	uint16_t GetArrayCount() const;
};


class Texture3D : public GenericTextureBase {
public:
	Texture3D(){}
	Texture3D(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, uint16_t depth, gxapi::eFormat format);

	uint64_t GetHeight() const;
	uint16_t GetDepth() const;
};


class TextureCubeMap : public GenericTextureBase {
public:
	TextureCubeMap(){}
	TextureCubeMap(gxapi::IGraphicsApi* graphicsApi, uint64_t width, uint32_t height, gxapi::eFormat format);

	uint64_t GetHeight() const;
};

//==================================

} // namespace gxeng
} // namespace inl
