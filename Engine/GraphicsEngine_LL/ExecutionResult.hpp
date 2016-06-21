#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"


namespace inl {
namespace gxeng {


class ExecutionResult {
public:
	ExecutionResult();
	~ExecutionResult();

	void AddCommandList(GraphicsCommandList&& list, float aluVsBandwidthHeavy = 0.5);
	void AddCommandList(ComputeCommandList&& list, float aluVsBandwidthHeavy = 0.5);
	void AddCommandList(CopyCommandList&& list);
};


} // namespace gxeng
} // namespace inl
