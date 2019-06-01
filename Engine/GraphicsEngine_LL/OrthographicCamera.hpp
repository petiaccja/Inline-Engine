#pragma once

#include "BasicCamera.hpp"

#include <GraphicsEngine/Scene/IOrthographicCamera.hpp>

#include <InlineMath.hpp>


namespace inl::gxeng {


class OrthographicCamera : virtual public BasicCamera, virtual public IOrthographicCamera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};

public:
	OrthographicCamera();

	// Set Frustum bounds the traditional way. Will overwrite view transform too.
	void SetBounds(float left, float right, float bottom, float top, float zNear, float zFar) override;

	//negative values are accepted and will result in a flipped image
	void SetWidth(float width) override;
	void SetHeight(float height) override;

	float GetWidth() const override;
	float GetHeight() const override;

	float GetAspectRatio() const override;

	// Matrices
	Mat44 GetViewMatrix() const override;
	Mat44 GetProjectionMatrix() const override;
	Mat44 GetPrevViewMatrix() const override;

private:
	float m_width;
	float m_height;
};


} // namespace inl::gxeng
