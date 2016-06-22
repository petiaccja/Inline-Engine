#include "ComputeCommandList.hpp"


namespace inl {
namespace gxeng {



ComputeCommandList::ComputeCommandList(CommandAllocatorPool& cmdAllocatorPool)
	: CopyCommandList(cmdAllocatorPool, gxapi::eCommandListType::COMPUTE)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(GetCommandList());
}

ComputeCommandList::ComputeCommandList(CommandAllocatorPool& cmdAllocatorPool, gxapi::eCommandListType type)
	: CopyCommandList(cmdAllocatorPool, type)
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



} // namespace gxeng
} // namespace inl
