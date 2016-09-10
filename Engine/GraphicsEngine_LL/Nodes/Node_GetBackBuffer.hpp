#pragma once

#include "../GraphicsNode.hpp"

#include <cmath>


namespace inl {
namespace gxeng {
namespace nodes {


class GetBackBuffer :
	virtual public GraphicsNode,
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<Texture2D*>
{
public:
	GetBackBuffer() {}

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override {
		return Task({ [this](const ExecutionContext& context)
		{
			this->GetOutput<0>().Set(static_cast<const SwapChainAccessContext&>(context).GetBackBuffer());
			return ExecutionResult{};
		} });
	}
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
