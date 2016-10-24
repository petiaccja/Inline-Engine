#include "Node_GetDepthBuffer.hpp"

#include "../ResouceViewFactory.hpp"

namespace inl {
namespace gxeng {
namespace nodes {


GetDepthBuffer::GetDepthBuffer(MemoryManager* memgr, ResourceViewFactory* resViewFactory, unsigned width, unsigned height) {
	m_memoryManager = memgr;
	m_resourceViewFactory = resViewFactory;

	Init(width, height);
}


void GetDepthBuffer::Init(unsigned width, unsigned height) {
	auto resource = ToShared(m_memoryManager->CreateTexture2D(eResourceHeapType::CRITICAL, width, height, gxapi::eFormat::D32_FLOAT, gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL));
	gxapi::DsvTexture2DArray desc;
	desc.activeArraySize = 1;
	desc.firstArrayElement = 0;
	desc.firstMipLevel = 0;
	m_dsv = m_resourceViewFactory->CreateDepthStencilView(resource, desc);
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
