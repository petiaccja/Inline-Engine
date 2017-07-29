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
	Mat44 GetViewMatrix() const override;
	Mat44 GetProjectionMatrix() const override;
	Mat44 GetPrevViewMatrix() const override;

protected:
	float m_fovH;
	float m_fovV;
};


} // namespace inl::gxeng
