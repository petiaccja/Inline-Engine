#include "BackBufferManager.hpp"

#include "MemoryObject.hpp"
#include "ResourceView.hpp"
#include "HostDescHeap.hpp"

#include <cassert>

namespace inl {
namespace gxeng {


BackBufferManager::BackBufferManager(gxapi::IGraphicsApi* graphicsApi, gxapi::ISwapChain* swapChain) :
	m_graphicsApi(graphicsApi),
	m_swapChain(swapChain)
{
	const unsigned numBuffers = swapChain->GetDesc().numBuffers;

	m_backBuffers.reserve(numBuffers);
	for (unsigned i = 0; i < numBuffers; i++) {
		MemoryObject::UniquePtr resource(swapChain->GetBuffer(i), std::default_delete<const gxapi::IResource>());
		gxapi::ResourceDesc resourceDesc = resource->GetDesc();

		Texture2D texture{ std::move(resource), true, eResourceHeap::BACKBUFFER };
		m_backBuffers.push_back(std::move(texture));
	}
}


Texture2D& BackBufferManager::GetBackBuffer(unsigned index) {
	return m_backBuffers.at(index);
}


} // namespace gxeng
} // namespace inl
