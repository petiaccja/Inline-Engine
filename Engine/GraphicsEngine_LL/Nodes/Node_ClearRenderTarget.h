#pragma once

#include "../GraphicsNode.hpp"

namespace inl {
namespace gxeng {
namespace nodes {


class ClearRenderTarget :
	public virtual GraphicsNode,
	public exc::InputPortConfig<BackBuffer*, gxapi::ColorRGBA>,
	public exc::OutputPortConfig<BackBuffer*>
{
public:
	ClearRenderTarget() {
		this->GetInput<0>().Clear();
	}

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override;
};


} // namespace nodes
} // namespace gxeng
} // namespace inl
