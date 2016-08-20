#include "GraphicsCommandList.hpp"


namespace inl {
namespace gxeng {


GraphicsCommandList::GraphicsCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool)
	: ComputeCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::GRAPHICS)
{
	m_commandList = dynamic_cast<gxapi::IGraphicsCommandList*>(GetCommandList());
}


GraphicsCommandList::GraphicsCommandList(GraphicsCommandList&& rhs)
	: ComputeCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList)
{
	rhs.m_commandList = nullptr;
}


GraphicsCommandList& GraphicsCommandList::operator=(GraphicsCommandList&& rhs) {
	ComputeCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}


BasicCommandList::Decomposition GraphicsCommandList::Decompose() {
	m_commandList = nullptr;

	return ComputeCommandList::Decompose();
}



} // namespace gxeng
} // namespace inl