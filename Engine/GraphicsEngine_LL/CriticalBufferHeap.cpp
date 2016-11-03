#include "CriticalBufferHeap.hpp"

#include "MemoryObject.hpp"
#include "CopyCommandList.hpp"

#include <iostream>


namespace inl {
namespace gxeng {
namespace impl {



CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi * graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


MemoryObjectDescriptor CriticalBufferHeap::Allocate(gxapi::ResourceDesc desc, gxapi::ClearValue* clearValue) {
	MemoryObjectDescriptor result;

	result.resource = m_graphicsApi->CreateCommittedResource(
		gxapi::HeapProperties(gxapi::eHeapType::DEFAULT),
		gxapi::eHeapFlags::NONE,
		desc,
		gxapi::eResourceState::COMMON,
		clearValue
	);

	result.deleter = std::default_delete<gxapi::IResource>();
	result.resident = true;

	return result;
}


} // namespace impl
} // namespace gxeng
} // namespace inl
