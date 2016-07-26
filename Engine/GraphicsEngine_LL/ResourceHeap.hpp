#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include <unordered_map>

namespace inl {
namespace gxeng {


class GenericResource;

enum class eResourceHeapType { CRITICAL };


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


#if 0 //TODO how can different high-level resources \
use the same underlying resource when they need different descriptors?

class OverlappedBufferHeap final : public BasicHeap {
public:
	//gxapi::IResource* Allocate(GenericResource* owner, gxapi::ResourceDesc desc, gxapi::IResource* shared = nullptr);
	void ReleaseUnderlying(GenericResource* owner) override;
};
#endif


} // impl
} // gxeng
} // inl
