#include "OrthographicCamera.hpp"

namespace inl::gxeng {

OrthographicCamera::OrthographicCamera():
	m_width(2),
	m_height(2)
{}


void OrthographicCamera::SetBounds(float left, float right, float bottom, float top, float zNear, float zFar) {
	m_width = right - left;
	m_height = top - bottom;
	m_nearPlane = zNear;
	m_farPlane = zFar;

	m_position = mathfu::Vector3f(left + m_width*0.5f, bottom + m_height*0.5f, 0.f);
	m_lookdir = mathfu::Vector3f(0, 0, -1);
	m_upVector = mathfu::Vector3f(0, 1, 0);
	m_targetDistance = 1.f;
}

void OrthographicCamera::SetWidth(float width) {
	m_width = width;
}


void OrthographicCamera::SetHeight(float height) {
	m_height = height;
}


float OrthographicCamera::GetWidth() const {
	return m_width;
}

float OrthographicCamera::GetHeight() const {
	return m_height;
}

float OrthographicCamera::GetAspectRatio() const {
	return std::abs(m_width / m_height);
}

mathfu::Matrix4x4f OrthographicCamera::GetViewMatrixRH() const {
	return mathfu::Matrix4x4f::LookAt(m_position + m_lookdir, m_position, m_upVector, +1.0f);
}

mathfu::Matrix4x4f OrthographicCamera::GetViewMatrixLH() const {
	return mathfu::Matrix4x4f::LookAt(m_position + m_lookdir, m_position, m_upVector, -1.0f);
}

mathfu::Matrix4x4f OrthographicCamera::GetProjectionMatrixRH() const {
	const float widthHalf = m_width*0.5f;
	const float heightHalf = m_height*0.5f;
	return mathfu::Matrix4x4f::Ortho(-widthHalf, widthHalf, -heightHalf, heightHalf, m_nearPlane, m_farPlane, +1.f);
}

mathfu::Matrix4x4f OrthographicCamera::GetProjectionMatrixLH() const {
	const float widthHalf = m_width*0.5f;
	const float heightHalf = m_height*0.5f;
	return mathfu::Matrix4x4f::Ortho(-widthHalf, widthHalf, -heightHalf, heightHalf, m_nearPlane, m_farPlane, -1.f);
}





}
