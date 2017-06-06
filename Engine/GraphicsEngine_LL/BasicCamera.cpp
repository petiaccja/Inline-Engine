#include "BasicCamera.hpp"

#include <string>

namespace inl::gxeng {



BasicCamera::BasicCamera():
	m_name("unnamed"),
	m_position(0, -1, 0),
	m_upVector(0, 0, 1),
	m_lookdir(0, 1, 0),
	m_prevPosition(m_position),
	m_prevUpVector(m_upVector),
	m_prevLookdir(m_lookdir),
	m_targetDistance(1),
	m_focus(1.f),
	m_nearPlane(0.1f),
	m_farPlane(100.0f)
{}

void BasicCamera::SetName(std::string name) {
	m_name = name;
}


const std::string & BasicCamera::GetName() const {
	return m_name;
}


// Set positional properties.
void BasicCamera::SetPosition(mathfu::Vector3f position) {
	m_prevPosition = m_position;
	m_prevLookdir = m_lookdir;
	//m_prevUpVector = m_upVector;

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
void BasicCamera::SetTarget(mathfu::Vector3f targetPosition) {
	m_prevPosition = m_position;
	m_prevLookdir = m_lookdir;
	//m_prevUpVector = m_upVector;

	m_lookdir = targetPosition - m_position;
	m_targetDistance = m_lookdir.Length();
	m_lookdir.Normalize();
}
void BasicCamera::SetLookDirection(mathfu::Vector3f lookDirection) {
	//m_prevPosition = m_position;
	m_prevLookdir = m_lookdir;
	//m_prevUpVector = m_upVector;

	m_lookdir = lookDirection.Normalized();
}
void BasicCamera::SetUpVector(mathfu::Vector3f upVector) {
	//m_prevPosition = m_position;
	//m_prevLookdir = m_lookdir;
	m_prevUpVector = m_upVector;

	m_upVector = upVector.Normalized();
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
mathfu::Vector3f BasicCamera::GetPosition() const {
	return m_position;
}
mathfu::Vector3f BasicCamera::GetLookDirection() const {
	return m_lookdir;
}
mathfu::Vector3f BasicCamera::GetTarget() const {
	return m_position + m_lookdir * m_targetDistance;
}
float BasicCamera::GetTargetDistance() const {
	return m_targetDistance;
}
mathfu::Vector3f BasicCamera::GetUpVector() const {
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


}
