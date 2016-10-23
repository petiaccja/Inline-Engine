#include "Node_GetDepthBuffer.hpp"

#include "../ResouceViewFactory.hpp"

namespace inl {
namespace gxeng {
namespace nodes {


GetDepthBuffer::GetDepthBuffer(MemoryManager* memgr, ResourceViewFactory* resViewFactory, unsigned width, unsigned height, size_t bufferCount) :
	m_dsvs(0),
	currBuffer(0)
{
	for (int i = 0; i < bufferCount; i++) {
		auto resource = ToShared(memgr->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, gxapi::eFormat::D32_FLOAT, gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL));
		gxapi::DsvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;
		m_dsvs.push_back(resViewFactory->CreateDepthStencilView(resource, desc));
	}
}


Task GetDepthBuffer::GetTask() {
	return Task(
		{
			[this](const ExecutionContext& context) {
				currBuffer = (++currBuffer) % m_dsvs.size();
				this->GetOutput<0>().Set(m_dsvs[currBuffer]);
				return ExecutionResult{};
			}
		}
	);
}



} // namespace nodes
} // namespace gxeng
} // namespace inl
