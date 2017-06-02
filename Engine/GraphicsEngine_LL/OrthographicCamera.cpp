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

	m_position = Vec3(left + m_width*0.5f, bottom + m_height*0.5f, 0.f);
	m_lookdir = Vec3(0, 0, -1);
	m_upVector = Vec3(0, 1, 0);
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

Mat44 OrthographicCamera::GetViewMatrix() const {
	return Mat44::LookAt(m_position + m_lookdir, m_position, m_upVector, false, false);
}

Mat44 OrthographicCamera::GetViewMatrixLH() const {
	return Mat44::LookAt(m_position + m_lookdir, m_position, m_upVector, true, false);
}

Mat44 OrthographicCamera::GetProjectionMatrix() const {
	const float widthHalf = m_width*0.5f;
	const float heightHalf = m_height*0.5f;
	return Mat44::Orthographic({ -widthHalf, -heightHalf, -m_nearPlane }, { widthHalf, heightHalf, -m_farPlane }, 0, 1);
}

Mat44 OrthographicCamera::GetProjectionMatrixLH() const {
	const float widthHalf = m_width*0.5f;
	const float heightHalf = m_height*0.5f;
	return Mat44::Orthographic({ -widthHalf, -heightHalf, m_nearPlane }, { widthHalf, heightHalf, m_farPlane }, 0, 1);
}





}
