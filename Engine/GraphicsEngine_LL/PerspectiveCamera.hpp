#pragma once

#include "BasicCamera.hpp"

#include <string>
#include <InlineMath.hpp>


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
	Mat44 GetViewMatrixRH() const override;
	Mat44 GetViewMatrixLH() const override;
	Mat44 GetProjectionMatrixRH() const override;
	Mat44 GetProjectionMatrixLH() const override;

protected:
	float m_fovH;
	float m_fovV;
};


} // namespace inl::gxeng
