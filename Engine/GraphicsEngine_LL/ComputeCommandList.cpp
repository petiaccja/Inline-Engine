#include "ComputeCommandList.hpp"


namespace inl {
namespace gxeng {



ComputeCommandList::ComputeCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool)
	: CopyCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::COMPUTE)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(GetCommandList());
}

ComputeCommandList::ComputeCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool, gxapi::eCommandListType type)
	: CopyCommandList(gxApi, commandAllocatorPool, scratchSpacePool, type)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(GetCommandList());
}


ComputeCommandList::ComputeCommandList(ComputeCommandList&& rhs)
	: CopyCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList)
{
	rhs.m_commandList = nullptr;
}


ComputeCommandList& ComputeCommandList::operator=(ComputeCommandList&& rhs) {
	CopyCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}


BasicCommandList::Decomposition ComputeCommandList::Decompose() {
	m_commandList = nullptr;
	
	return CopyCommandList::Decompose();
}


// Command list state
void ComputeCommandList::ResetState(gxapi::IPipelineState* newState) {
	m_commandList->ResetState(newState);
}

void ComputeCommandList::SetPipelineState(gxapi::IPipelineState* pipelineState) {
	m_commandList->SetPipelineState(pipelineState);
}


} // namespace gxeng
} // namespace inl
