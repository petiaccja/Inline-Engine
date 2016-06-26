#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"


namespace inl {
namespace gxeng {



class ExecutionContext {
public:
	ExecutionContext(CommandAllocatorPool& commandAllocatorPool);
	

	GraphicsCommandList GetGraphicsCommandList();
	ComputeCommandList GetComputeCommandList();
	CopyCommandList GetCopyCommandList();
private:
	CommandAllocatorPool* m_commandAllocatorPool;
};



} // namespace gxeng
} // namespace inl