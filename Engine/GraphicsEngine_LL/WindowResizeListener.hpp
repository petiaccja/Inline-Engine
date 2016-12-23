#pragma once

namespace inl::gxeng {


class WindowResizeListener {
public:
	virtual void WindowResized(unsigned width, unsigned height) = 0;
};


} // inl::gxeng
