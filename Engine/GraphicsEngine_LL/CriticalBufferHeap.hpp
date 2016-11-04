#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"

#include "MemoryObject.hpp"

namespace inl {
namespace gxeng {

namespace impl {

class CriticalBufferHeap {
public:
	CriticalBufferHeap(gxapi::IGraphicsApi* graphicsApi);
	MemoryObjDesc Allocate(gxapi::ResourceDesc desc, gxapi::ClearValue* clearValue = nullptr);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
};


} // namespace impl
} // namespace gxeng
} // namespace inl
