#include "VolatileViewHeap.hpp"


namespace inl :: gxeng {


VolatileViewHeap::VolatileViewHeap(gxapi::IGraphicsApi* graphicsApi) :
	m_graphicsApi(graphicsApi),
	m_nextPos(0)
{}


gxapi::DescriptorHandle VolatileViewHeap::Allocate() {
	size_t heapId = m_nextPos / HEAP_SIZE;
	size_t descriptorIndex = m_nextPos % HEAP_SIZE;
	if (heapId >= m_heaps.size()) {
		m_heaps.push_back(
			std::unique_ptr<gxapi::IDescriptorHeap>(
				m_graphicsApi->CreateDescriptorHeap({ gxapi::eDescriptorHeapType::CBV_SRV_UAV, HEAP_SIZE, false })
			)
		);
	}
	m_nextPos += 1;
	return m_heaps[heapId]->At(descriptorIndex);
}


} // namespace gxeng
