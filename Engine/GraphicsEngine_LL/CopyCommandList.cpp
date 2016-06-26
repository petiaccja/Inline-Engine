#include "CopyCommandList.hpp"

namespace inl {
namespace gxeng {


CopyCommandList::CopyCommandList(CommandAllocatorPool& cmdAllocatorPool) 
	: BasicCommandList(cmdAllocatorPool, gxapi::eCommandListType::COPY)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(CommandAllocatorPool& cmdAllocatorPool, gxapi::eCommandListType type) 
	: BasicCommandList(cmdAllocatorPool, type)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(CopyCommandList&& rhs)
	: BasicCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList) 
{
	rhs.m_commandList = nullptr;
}


CopyCommandList& CopyCommandList::operator=(CopyCommandList&& rhs) {
	BasicCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}


BasicCommandList::Decomposition CopyCommandList::Decompose() {
	m_commandList = nullptr;
	return BasicCommandList::Decompose();
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