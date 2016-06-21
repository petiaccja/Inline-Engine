#include "CopyCommandList.hpp"

namespace inl {
namespace gxeng {


CopyCommandList::CopyCommandList(CommandAllocatorPool& cmdAllocatorPool) 
	: BasicCommandList(cmdAllocatorPool, gxapi::eCommandListType::COPY)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(BasicCommandList::m_commandList.get());
}

CopyCommandList::CopyCommandList(CommandAllocatorPool& cmdAllocatorPool, gxapi::eCommandListType type) 
	: BasicCommandList(cmdAllocatorPool, type)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(BasicCommandList::m_commandList.get());
}


// Command list state
void CopyCommandList::ResetState(gxapi::IPipelineState* newState) {
	m_commandList->ResetState(newState);
}



// resource copy
// TODO


// barriers
// TODO




} // namespace gxeng
} // namespace inl