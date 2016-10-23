#pragma once

#include "../GraphicsNode.hpp"

#include <cmath>


namespace inl {
namespace gxeng {
namespace nodes {


class GetTime :
	virtual public GraphicsNode,
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<double>
{
public:
	GetTime() {}

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override {
		return Task({ [this](const ExecutionContext& context)
		{
			std::chrono::nanoseconds time = context.GetAbsoluteTime();
			double dtime = time.count() / 1e9;
			this->GetOutput<0>().Set(dtime);
			return ExecutionResult{};
		} });
	}
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
