#include "Camera2D.hpp"


namespace inl::gxeng {


Camera2D::Camera2D() {
	m_position = { 0,0 };
	m_extent = { 1,1 };
	m_rotation = 0;
}


void Camera2D::SetName(std::string name) {
	m_name = name;
}


const std::string& Camera2D::GetName() const {
	return m_name;
}


void Camera2D::SetPosition(Vec2 position) {
	m_position = position;
}


void Camera2D::SetRotation(float rotation) {
	m_rotation = rotation;
}


void Camera2D::SetVerticalFlip(bool enable) {
	m_verticalFlip = enable;
}


Vec2 Camera2D::GetPosition() const {
	return m_position;
}


float Camera2D::GetRotation() const {
	return m_rotation;
}


void Camera2D::SetExtent(Vec2 extent) {
	m_extent = extent;
}

Vec2 Camera2D::GetExtent() const {
	return m_extent;
}

bool Camera2D::GetVerticalFlip() const {
	return m_verticalFlip;
}


Mat33 Camera2D::GetViewMatrix() const {
	Vec2 target = Vec2{0.0f, 1.0f}*Mat22::Rotation(m_rotation) + m_position;
	Mat33 view = Mat33::LookAt(m_position, target, !m_verticalFlip, false);
	return view;
}


Mat33 Camera2D::GetProjectionMatrix() const {
	Mat33 proj = Mat33::Orthographic(-m_extent/2, m_extent/2, -1.f, 1.0f);
	return proj;
}


} // namespace inl::gxeng