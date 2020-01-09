#pragma once

//#include "HostDescHeap.hpp"
#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IDescriptorHeap.hpp"

namespace inl :: gxeng {


/// <summary>
/// This class provides an abstraction over descriptor heaps to
/// allow a pipeline node to easily create views for volatile resources
/// like a volatile constant buffer.
/// <para/>
/// This class is NOT thread safe.
/// </summary>
class VolatileViewHeap {
public:
	VolatileViewHeap(gxapi::IGraphicsApi* graphicsApi);

	gxapi::DescriptorHandle Allocate();

private:
	static constexpr size_t HEAP_SIZE = 128;

	gxapi::IGraphicsApi* m_graphicsApi;
	size_t m_nextPos;
	std::vector<std::unique_ptr<gxapi::IDescriptorHeap>> m_heaps;
};


} // namespace gxeng
