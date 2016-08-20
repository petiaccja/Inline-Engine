#include "ExecutionContext.hpp"



namespace inl {
namespace gxeng {



ExecutionContext::ExecutionContext(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool) 
	: m_gxApi(gxApi), m_commandAllocatorPool(&commandAllocatorPool), m_scratchSpacePool(&scratchSpacePool)
{}


GraphicsCommandList ExecutionContext::GetGraphicsCommandList() const {
	return GraphicsCommandList(m_gxApi, *m_commandAllocatorPool, *m_scratchSpacePool);
}

ComputeCommandList ExecutionContext::GetComputeCommandList() const {
	return ComputeCommandList(m_gxApi, *m_commandAllocatorPool, *m_scratchSpacePool);
}

CopyCommandList ExecutionContext::GetCopyCommandList() const {
	return CopyCommandList(m_gxApi, *m_commandAllocatorPool, *m_scratchSpacePool);
}



} // namespace gxeng
} // namespace inl