#include "Camera.hpp"


namespace inl::gxeng {


Camera::Camera()
	: m_position(0, -1, 0),
	m_upVector(0, 0, 1),
	m_lookdir(0, 1, 0),
	m_targetDistance(1),
	m_fovH(60.f / 180.f*3.14159f),
	m_fovV(45.f / 180.f*3.14159f),
	m_focus(1.f),
	m_name("unnamed")
{}

void Camera::SetName(std::string name) {
	m_name = name;
}
const std::string& Camera::GetName() const {
	return m_name;
}

// Set positional properties.
void Camera::SetPosition(mathfu::Vector3f position) {
	if (!m_targeted) {
		m_position = position;
	}
	else {
		mathfu::Vector3f target = GetTarget();
		m_position = position;
		m_lookdir = target - m_position;
		m_targetDistance = m_lookdir.Length();
		m_lookdir.Normalize();
	}
}
void Camera::SetTarget(mathfu::Vector3f targetPosition) {
	m_lookdir = targetPosition - m_position;
	m_targetDistance = m_lookdir.Length();
	m_lookdir.Normalize();
}
void Camera::SetLookDirection(mathfu::Vector3f lookDirection) {
	m_lookdir = lookDirection.Normalized();
}
void Camera::SetUpVector(mathfu::Vector3f upVector) {
	m_upVector = upVector.Normalized();
}
void Camera::SetTargeted(bool targeted) {
	m_targeted = targeted;
}

// Set rendering properties.
void Camera::SetFocus(float focusDistance) {
	m_focus = focusDistance;
}
void Camera::SetFOVAspect(float horizontalFov, float aspectRatio) {
	m_fovH = horizontalFov;
	m_fovV = m_fovH / aspectRatio;
}
void Camera::SetFOVAxis(float horizontalFov, float verticalFov) {
	m_fovH = horizontalFov;
	m_fovV = verticalFov;
}

// Get positional properties.
mathfu::Vector3f Camera::GetPosition() const {
	return m_position;
}
mathfu::Vector3f Camera::GetLookDirection() const {
	return m_lookdir;
}
mathfu::Vector3f Camera::GetTarget() const {
	return m_position + m_lookdir * m_targetDistance;
}
float Camera::GetTargetDistance() const {
	return m_targetDistance;
}
mathfu::Vector3f Camera::GetUpVector() const {
	return m_upVector;
}

// Get rendering properties.
float Camera::GetFOVVertical() const {
	return m_fovV;
}
float Camera::GetFOVHorizontal() const {
	return m_fovH;
}
float Camera::GetAspectRatio() const {
	return m_fovH / m_fovV;
}
float Camera::GetFocus() const {
	return m_focus;
}



// Matrices
mathfu::Matrix4x4f Camera::GetViewMatrixRH() const {
	return mathfu::Matrix4x4f::LookAt(m_position + m_lookdir, m_position, m_upVector, +1.0f);
}
mathfu::Matrix4x4f Camera::GetViewMatrixLH() const {
	return mathfu::Matrix4x4f::LookAt(m_position + m_lookdir, m_position, m_upVector, -1.0f);
}
mathfu::Matrix4x4f Camera::GetPerspectiveMatrixRH(float nearPlane, float farPlane) const {
	return mathfu::Matrix4x4f::Perspective(m_fovV, m_fovH / m_fovV, nearPlane, farPlane, +1.0f);
}
mathfu::Matrix4x4f Camera::GetPerspectiveMatrixLH(float nearPlane, float farPlane) const {
	return mathfu::Matrix4x4f::Perspective(m_fovV, m_fovH / m_fovV, nearPlane, farPlane, -1.0f);
}
mathfu::Matrix4x4f Camera::GetOrthographicMatrixRH(float nearPlane, float farPlane) const {
	return mathfu::Matrix4x4f::Ortho(-m_fovH / 2, m_fovH / 2, -m_fovV / 2, m_fovV / 2, nearPlane, farPlane, +1.0f);
}
mathfu::Matrix4x4f Camera::GetOrthographicMatrixLH(float nearPlane, float farPlane) const {
	return mathfu::Matrix4x4f::Ortho(-m_fovH / 2, m_fovH / 2, -m_fovV / 2, m_fovV / 2, nearPlane, farPlane, -1.0f);
}



} // namespace inl::gxeng