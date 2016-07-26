#include "ResourceHeap.hpp"

#include "GpuBuffer.hpp"


namespace inl {
namespace gxeng {
namespace impl {


CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi * graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


gxapi::IResource* CriticalBufferHeap::Allocate(GenericResource* owner, gxapi::ResourceDesc desc) {
	gxapi::IResource* retval;
	{
		std::unique_ptr<gxapi::IResource> allocation(
			m_graphicsApi->CreateCommittedResource(
				gxapi::HeapProperties(gxapi::eHeapType::DEFAULT),
				gxapi::eHeapFlags::NONE,
				desc,
				gxapi::eResourceState::COMMON
			)
		);

		retval = allocation.get();

		//After the insertion has occured, no exception should leave uncatched
		m_resources.insert({owner, std::move(allocation)});
	}

	return retval;
}


void CriticalBufferHeap::ReleaseUnderlying(GenericResource* owner) {
	m_resources.erase(owner);
}


} // impl
} // gxeng
} // inl
