#include "ComputeCommandList.hpp"


namespace inl {
namespace gxeng {



ComputeCommandList::ComputeCommandList(CommandAllocatorPool& cmdAllocatorPool)
	: CopyCommandList(cmdAllocatorPool, gxapi::eCommandListType::COMPUTE)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(BasicCommandList::m_commandList.get());
}

ComputeCommandList::ComputeCommandList(CommandAllocatorPool& cmdAllocatorPool, gxapi::eCommandListType type)
	: CopyCommandList(cmdAllocatorPool, type)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(BasicCommandList::m_commandList.get());
}



} // namespace gxeng
} // namespace inl
