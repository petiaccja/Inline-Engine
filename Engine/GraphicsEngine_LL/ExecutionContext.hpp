#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"


namespace inl {
namespace gxeng {



class ExecutionContext {
public:
	ExecutionContext(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool);


	GraphicsCommandList GetGraphicsCommandList() const;
	ComputeCommandList GetComputeCommandList() const;
	CopyCommandList GetCopyCommandList() const;
private:
	gxapi::IGraphicsApi* m_gxApi;
	CommandAllocatorPool* m_commandAllocatorPool;
	ScratchSpacePool* m_scratchSpacePool;
};



} // namespace gxeng
} // namespace inl