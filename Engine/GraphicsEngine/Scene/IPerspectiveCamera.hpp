#pragma once

#include <InlineMath.hpp>
#include "IBasicCamera.hpp"


namespace inl::gxeng {


class IPerspectiveCamera : virtual public IBasicCamera {
public:
	virtual ~IPerspectiveCamera() {}

	// Set rendering properties.
	virtual void SetFOVAspect(float horizontalFov, float aspectRatio) = 0;
	virtual void SetFOVAxis(float horizontalFov, float verticalFov) = 0;

	// Get rendering properties.
	virtual float GetFOVVertical() const = 0;
	virtual float GetFOVHorizontal() const = 0;
};



} // namespace inl::gxeng