#include "GraphicsCommandList.hpp"


namespace inl {
namespace gxeng {


GraphicsCommandList::GraphicsCommandList(CommandAllocatorPool& cmdAllocatorPool)
	: ComputeCommandList(cmdAllocatorPool, gxapi::eCommandListType::GRAPHICS)
{
	m_commandList = dynamic_cast<gxapi::IGraphicsCommandList*>(BasicCommandList::m_commandList.get());
}



} // namespace gxeng
} // namespace inl