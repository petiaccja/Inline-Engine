#include "Node_GetDepthBuffer.hpp"


namespace inl {
namespace gxeng {
namespace nodes {


GetDepthBuffer::GetDepthBuffer(MemoryManager* memgr, DSVHeap& dsvHeap, unsigned width, unsigned height):
	m_memoryManager(memgr),
	m_dsvHeap(dsvHeap)
{
	Init(width, height);
}


void GetDepthBuffer::Init(unsigned width, unsigned height) {
	auto resource = std::make_shared<Texture2D>(m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, gxapi::eFormat::D32_FLOAT, gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL));
	gxapi::DsvTexture2DArray desc;
	desc.activeArraySize = 1;
	desc.firstArrayElement = 0;
	desc.firstMipLevel = 0;
	m_dsv = DepthStencilView(resource, m_dsvHeap, desc);
}


void GetDepthBuffer::Resize(unsigned width, unsigned height) {
	Init(width, height);
}



Task GetDepthBuffer::GetTask() {
	return Task(
	{
		[this](const ExecutionContext& context) {
			this->GetOutput<0>().Set(m_dsv);
			return ExecutionResult{};
		}
	}
	);
}



} // namespace nodes
} // namespace gxeng
} // namespace inl
