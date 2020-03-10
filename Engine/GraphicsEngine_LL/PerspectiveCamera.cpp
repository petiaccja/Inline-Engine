#include "PerspectiveCamera.hpp"


namespace inl::gxeng {


PerspectiveCamera::PerspectiveCamera() : m_fovH(60.f / 180.f * 3.14159f),
										 m_fovV(45.f / 180.f * 3.14159f) {}


void PerspectiveCamera::SetFOVAspect(float horizontalFov, float aspectRatio) {
	m_fovH = horizontalFov;
	m_fovV = m_fovH / aspectRatio;
}
void PerspectiveCamera::SetFOVAxis(float horizontalFov, float verticalFov) {
	m_fovH = horizontalFov;
	m_fovV = verticalFov;
}


// Get rendering properties.
float PerspectiveCamera::GetFOVVertical() const {
	return m_fovV;
}
float PerspectiveCamera::GetFOVHorizontal() const {
	return m_fovH;
}
float PerspectiveCamera::GetAspectRatio() const {
	return m_fovH / m_fovV;
}


// Matrices
Mat44 PerspectiveCamera::GetViewMatrix() const {
	return LookAt(m_position, m_position + m_lookdir, m_upVector, true, false, false);
}
Mat44 PerspectiveCamera::GetProjectionMatrix() const {
	return Perspective(m_fovH, m_fovH / m_fovV, m_nearPlane, m_farPlane, 0.f, 1.f);
}
Mat44 PerspectiveCamera::GetPrevViewMatrix() const {
	return LookAt(m_prevPosition, m_prevPosition + m_prevLookdir, m_prevUpVector, true, false, false);
}



} // namespace inl::gxeng