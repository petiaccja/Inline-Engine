#pragma once

#include "../GraphicsNode.hpp"

namespace inl {
namespace gxeng {


class ClearScreen : 
	public virtual GraphicsNode, 
	public exc::InputPortConfig<gxapi::ColorRGBA>,
	public exc::OutputPortConfig<>
{
public:
	ClearScreen() {
	}

	virtual void Update() override {}

	virtual void Notify(exc::InputPortBase* sender) override {}

	virtual Task GetTask() override;

	void SetTarget(Texture2D* target);
private:
	Texture2D* m_target = nullptr;
};


} // namespace gxeng
} // namespace inl
