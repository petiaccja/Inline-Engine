#include "BasicCamera.hpp"

#include <string>

namespace inl::gxeng {



BasicCamera::BasicCamera() : m_name("unnamed"),
							 m_position(0, -1, 0),
							 m_upVector(0, 0, 1),
							 m_lookdir(0, 1, 0),
							 m_prevPosition(m_position),
							 m_prevUpVector(m_upVector),
							 m_prevLookdir(m_lookdir),
							 m_targetDistance(1),
							 m_focus(1.f),
							 m_nearPlane(0.1f),
							 m_farPlane(100.0f) {}

void BasicCamera::SetName(std::string name) {
	m_name = name;
}


const std::string& BasicCamera::GetName() const {
	return m_name;
}


// Set positional properties.
void BasicCamera::SetPosition(Vec3 position) {
	m_prevPosition = m_position;
	m_prevLookdir = m_lookdir;
	//m_prevUpVector = m_upVector;

	if (!m_targeted) {
		m_position = position;
	}
	else {
		Vec3 target = GetTarget();
		m_position = position;
		m_lookdir = target - m_position;
		m_targetDistance = Length(m_lookdir);
		m_lookdir = Normalize(m_lookdir);
	}
}
void BasicCamera::SetTarget(Vec3 targetPosition) {
	m_prevPosition = m_position;
	m_prevLookdir = m_lookdir;

	//m_prevUpVector = m_upVector;
	m_lookdir = targetPosition - m_position;
	m_targetDistance = Length(m_lookdir);
	m_lookdir = Normalize(m_lookdir);
}
void BasicCamera::SetLookDirection(Vec3 lookDirection) {
	//m_prevPosition = m_position;
	m_prevLookdir = m_lookdir;
	//m_prevUpVector = m_upVector;

	m_lookdir = Normalize(lookDirection);
}
void BasicCamera::SetUpVector(Vec3 upVector) {
	//m_prevPosition = m_position;
	//m_prevLookdir = m_lookdir;
	m_prevUpVector = m_upVector;

	m_upVector = Normalize(upVector);
}
void BasicCamera::SetTargeted(bool targeted) {
	m_targeted = targeted;
}

// Set rendering properties.
void BasicCamera::SetFocus(float focusDistance) {
	m_focus = focusDistance;
}


// Set depth plane Z offset.
void BasicCamera::SetNearPlane(float zOffset) {
	m_nearPlane = zOffset;
}
void BasicCamera::SetFarPlane(float zOffset) {
	m_farPlane = zOffset;
}

// Get positional properties.
Vec3 BasicCamera::GetPosition() const {
	return m_position;
}
Vec3 BasicCamera::GetLookDirection() const {
	return m_lookdir;
}
Vec3 BasicCamera::GetTarget() const {
	return m_position + m_lookdir * m_targetDistance;
}
float BasicCamera::GetTargetDistance() const {
	return m_targetDistance;
}
Vec3 BasicCamera::GetUpVector() const {
	return m_upVector;
}


float BasicCamera::GetFocus() const {
	return m_focus;
}

// Get depth plane Z offset.
float BasicCamera::GetNearPlane() const {
	return m_nearPlane;
}
float BasicCamera::GetFarPlane() const {
	return m_farPlane;
}


} // namespace inl::gxeng
