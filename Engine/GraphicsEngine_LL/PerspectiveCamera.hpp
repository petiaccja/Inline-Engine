#pragma once

#include "BasicCamera.hpp"

#include <string>
#include <mathfu/mathfu_exc.hpp>


namespace inl::gxeng {


class PerspectiveCamera : public BasicCamera {
public:
	PerspectiveCamera();
	virtual ~PerspectiveCamera() {}

	// Set rendering properties.
	void SetFOVAspect(float horizontalFov, float aspectRatio);
	void SetFOVAxis(float horizontalFov, float verticalFov);

	// Get rendering properties.
	float GetFOVVertical() const;
	float GetFOVHorizontal() const;

	float GetAspectRatio() const override;

	// Matrices
	mathfu::Matrix4x4f GetViewMatrixRH() const override;
	mathfu::Matrix4x4f GetViewMatrixLH() const override;
	mathfu::Matrix4x4f GetProjectionMatrixRH() const override;
	mathfu::Matrix4x4f GetProjectionMatrixLH() const override;

protected:
	float m_fovH;
	float m_fovV;
};


} // namespace inl::gxeng
