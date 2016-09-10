#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"
#include "../GraphicsApi_LL/ISwapChain.hpp"

#include <unordered_map>

namespace inl {
namespace gxeng {

class CopyCommandList;
class GenericResource;
class LinearBuffer;
class Texture2D;

enum class eResourceHeapType { CRITICAL, UPLOAD };


namespace impl {


class BasicHeap {
public:
	virtual ~BasicHeap() = default;

	virtual void ReleaseUnderlying(GenericResource* owner) = 0;
};

#if 0 //TODO
class ConstantBufferHeap final : public BasicHeap {
public:
	ConstantBufferHeap(gxapi::IGraphicsApi* graphicsApi);

	gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc);
	void ReleaseUnderlying(GenericResource* owner) override;

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	static_assert(false, "TODO move const buffer implementation here!");
};
#endif


class CriticalBufferHeap final : public BasicHeap {
public:
	CriticalBufferHeap(gxapi::IGraphicsApi* graphicsApi);
	gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc);
	void ReleaseUnderlying(GenericResource* owner) override;

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::unordered_map<GenericResource*, std::unique_ptr<gxapi::IResource>> m_resources;
};


#if 0 //TODO
class StreamedBufferHeap final : public BasicHeap {
public:
	gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc);
	void ReleaseUnderlying(GenericResource* owner) override;
};
#endif


//TODO how can different high-level resources use the same underlying resource?
#if 0

class OverlappedBufferHeap final : public BasicHeap {
public:
	//gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc, gxapi::IResource* shared = nullptr);
	void ReleaseUnderlying(GenericResource* owner) override;
};
#endif


} // namespace impl


class BackBufferHeap
{
public:
	BackBufferHeap(gxapi::IGraphicsApi* graphicsApi, gxapi::ISwapChain* swapChain);

	Texture2D& GetBackBuffer(unsigned index);
	
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	gxapi::ISwapChain* m_swapChain;

	std::unique_ptr<gxapi::IDescriptorHeap> m_descriptorHeap;
	std::vector<Texture2D> m_backBuffers;
};


class UploadHeap {
public:
	UploadHeap(gxapi::IGraphicsApi* graphicsApi);

	void UploadToResource(gxeng::CopyCommandList& cmdList, LinearBuffer& target, const void* data, size_t size);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	std::vector<GenericResource> m_stagedResources;
};


} // namespace gxeng
} // namespace inl
