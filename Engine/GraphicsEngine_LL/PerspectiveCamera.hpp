#pragma once

#include "BasicCamera.hpp"

#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>

#include <InlineMath.hpp>


namespace inl::gxeng {


class PerspectiveCamera : virtual public BasicCamera, virtual public IPerspectiveCamera {
public:
	PerspectiveCamera();

	// Set rendering properties.
	void SetFOVAspect(float horizontalFov, float aspectRatio) override;
	void SetFOVAxis(float horizontalFov, float verticalFov) override;

	// Get rendering properties.
	float GetFOVVertical() const override;
	float GetFOVHorizontal() const override;

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
