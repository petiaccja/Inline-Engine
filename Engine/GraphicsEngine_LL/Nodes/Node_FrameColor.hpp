#pragma once

#include "../GraphicsNode.hpp"

#include <cmath>


namespace inl {
namespace gxeng {
namespace nodes {



class FrameColor :
	public exc::InputPortConfig<unsigned long long>,
	public exc::OutputPortConfig<gxapi::ColorRGBA>
{
public:
	FrameColor() {}

	virtual void Update() override {
		uint64_t x = GetInput<0>().Get();
		x %= 360;

		float r = sin(float(x) / 360.f * 2.f * 3.1415926f);
		r = 0.5f*r + 0.5f;

		float g = sin(float(x + 120) / 360.f * 2.f * 3.1415926f);
		g = 0.5f*g + 0.5f;

		float b = sin(float(x + 240) / 360.f * 2.f * 3.1415926f);
		b = 0.5f*b + 0.5f;

		GetOutput<0>().Set(gxapi::ColorRGBA(r, g, b));
	}

	virtual void Notify(exc::InputPortBase* sender) override {}
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
