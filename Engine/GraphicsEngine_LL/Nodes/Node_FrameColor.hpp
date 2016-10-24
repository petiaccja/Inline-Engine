#pragma once

#include "../GraphicsNode.hpp"

#include <cmath>


namespace inl {
namespace gxeng {
namespace nodes {



class FrameColor :
	public exc::InputPortConfig<double>,
	public exc::OutputPortConfig<gxapi::ColorRGBA>
{
public:
	FrameColor() {}

	virtual void Update() override {
		double x = GetInput<0>().Get();
		constexpr double pi = 3.141592653589793238;
		constexpr double frequency = 0.3;
		x *= frequency;

		float r = sin(x*2*pi);
		r = 0.5f*r + 0.5f;

		float g = sin(x*2*pi + 2*pi/3);
		g = 0.5f*g + 0.5f;

		float b = sin(x*2*pi + 4*pi/3);
		b = 0.5f*b + 0.5f;

		GetOutput<0>().Set(gxapi::ColorRGBA(r*0.1f + 0.3f, g*0.1f + 0.3f, b*0.1f + 0.3f));
	}

	virtual void Notify(exc::InputPortBase* sender) override {}
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
