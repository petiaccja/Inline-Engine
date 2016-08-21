#pragma once

#include "../GraphicsNode.hpp"

namespace inl {
namespace gxeng {


class ClearScreen : 
	public GraphicsNode, 
	public exc::InputPortConfig<>,
	public exc::OutputPortConfig<>
{
public:
	ClearScreen() {
	}

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override;

private:

};


} // namespace gxeng
} // namespace inl
