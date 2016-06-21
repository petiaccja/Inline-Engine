#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"


namespace inl {
namespace gxeng {



class ExecutionContext
{
public:
	ExecutionContext();
	~ExecutionContext();

	GraphicsCommandList GetGraphicsCommandList();
	ComputeCommandList GetComputeCommandList();
	CopyCommandList GetCopyCommandList();
};



} // namespace gxeng
} // namespace inl