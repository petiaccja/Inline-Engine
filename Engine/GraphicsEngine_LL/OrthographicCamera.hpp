#pragma once

#include "BasicCamera.hpp"

#include <string>
#include <mathfu/mathfu_exc.hpp>


namespace inl::gxeng {


class OrthographicCamera : public BasicCamera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};
public:
	OrthographicCamera();
	virtual ~OrthographicCamera() {}

	// Set Frustum bounds the traditional way. Will overwrite view transform too.
	void SetBounds(float left, float right, float bottom, float top, float zNear, float zFar);

	//negative values are accepted and will result in a flipped image
	void SetWidth(float width);
	void SetHeight(float height);

	float GetWidth() const;
	float GetHeight() const;

	float GetAspectRatio() const override;

	// Matrices
	mathfu::Matrix4x4f GetViewMatrixRH() const override;
	mathfu::Matrix4x4f GetViewMatrixLH() const override;
	mathfu::Matrix4x4f GetPrevViewMatrixRH() const override;
	mathfu::Matrix4x4f GetPrevViewMatrixLH() const override;
	mathfu::Matrix4x4f GetProjectionMatrixRH() const override;
	mathfu::Matrix4x4f GetProjectionMatrixLH() const override;

private:
	float m_width;
	float m_height;
};


} // namespace inl::gxeng
