#pragma once

#include "IBasicCamera.hpp"


namespace inl::gxeng {


class IOrthographicCamera : virtual public IBasicCamera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};

public:
	virtual ~IOrthographicCamera() {}

	// Set Frustum bounds the traditional way. Will overwrite view transform too.
	virtual void SetBounds(float left, float right, float bottom, float top, float zNear, float zFar) = 0;

	//negative values are accepted and will result in a flipped image
	virtual void SetWidth(float width) = 0;
	virtual void SetHeight(float height) = 0;

	virtual float GetWidth() const = 0;
	virtual float GetHeight() const = 0;
};


} // namespace inl::gxeng
