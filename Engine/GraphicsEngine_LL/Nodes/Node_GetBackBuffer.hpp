#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../PipelineTypes.hpp"

#include <cmath>


namespace inl::gxeng::nodes {


class GetBackBuffer :
	virtual public GraphicsNode,
	public GraphicsTask,
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<Texture2D>
{
public:
	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}

	void Setup(SetupContext& context) override {
		auto& swapChainAccessContext = static_cast<const SwapChainAccessContext&>(context);
		GetOutput<0>().Set(swapChainAccessContext.GetBackBuffer());
	}

	void Execute(RenderContext& context) override {}
};


} // namespace inl::gxeng::nodes
