#pragma once

#include <InlineMath.hpp>

#include <string>

namespace inl::gxeng {

/// <summary>
/// Abstract base class of perspective and orthographic cameras
/// </summary>
class BasicCamera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};
public:
	BasicCamera();
	virtual ~BasicCamera() = default;

	void SetName(std::string name);
	const std::string& GetName() const;

	// Set positional properties.
	void SetPosition(Vec3 position);
	void SetTarget(Vec3 targetPosition);
	void SetLookDirection(Vec3 lookDirection);
	void SetUpVector(Vec3 upVector);
	void SetTargeted(bool targeted);

	// Set rendering properties.
	void SetFocus(float focusDistance);

	// negative values are valid
	void SetNearPlane(float zOffset);
	void SetFarPlane(float zOffset);

	// Get positional properties.
	Vec3 GetPosition() const;
	Vec3 GetLookDirection() const;
	Vec3 GetTarget() const;
	float GetTargetDistance() const;
	Vec3 GetUpVector() const;

	// Get rendering properties.
	float GetFocus() const;

	// Get depth plane Z offset.
	float GetNearPlane() const;
	float GetFarPlane() const;

	virtual float GetAspectRatio() const = 0;

	virtual Mat44 GetViewMatrix() const = 0;
	virtual Mat44 GetProjectionMatrix() const = 0;
	virtual mathfu::Matrix4x4f GetPrevViewMatrixLH() const = 0;

protected:
	std::string m_name;

	Vec3 m_position;
	Vec3 m_upVector;
	Vec3 m_lookdir;
	mathfu::Vector3f m_prevPosition;
	mathfu::Vector3f m_prevUpVector;
	mathfu::Vector3f m_prevLookdir;

	float m_targetDistance;
	bool m_targeted = false;

	float m_focus;

	float m_nearPlane;
	float m_farPlane;
};


} // inl::gxeng
