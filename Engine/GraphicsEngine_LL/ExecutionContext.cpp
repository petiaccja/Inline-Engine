#include "ExecutionContext.hpp"



namespace inl {
namespace gxeng {



ExecutionContext::ExecutionContext(CommandAllocatorPool& commandAllocatorPool) 
	: m_commandAllocatorPool(&commandAllocatorPool)
{}


GraphicsCommandList ExecutionContext::GetGraphicsCommandList() {
	return GraphicsCommandList(*m_commandAllocatorPool);
}

ComputeCommandList ExecutionContext::GetComputeCommandList() {
	return ComputeCommandList(*m_commandAllocatorPool);
}

CopyCommandList ExecutionContext::GetCopyCommandList() {
	return CopyCommandList(*m_commandAllocatorPool);
}



} // namespace gxeng
} // namespace inl