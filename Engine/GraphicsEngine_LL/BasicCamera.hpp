#pragma once

#include <mathfu/mathfu_exc.hpp>

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
	void SetPosition(mathfu::Vector3f position);
	void SetTarget(mathfu::Vector3f targetPosition);
	void SetLookDirection(mathfu::Vector3f lookDirection);
	void SetUpVector(mathfu::Vector3f upVector);
	void SetTargeted(bool targeted);

	// Set rendering properties.
	void SetFocus(float focusDistance);

	// negative values are valid
	void SetNearPlane(float zOffset);
	void SetFarPlane(float zOffset);

	// Get positional properties.
	mathfu::Vector3f GetPosition() const;
	mathfu::Vector3f GetLookDirection() const;
	mathfu::Vector3f GetTarget() const;
	float GetTargetDistance() const;
	mathfu::Vector3f GetUpVector() const;

	// Get rendering properties.
	float GetFocus() const;

	// Get depth plane Z offset.
	float GetNearPlane() const;
	float GetFarPlane() const;

	virtual float GetAspectRatio() const = 0;

	virtual mathfu::Matrix4x4f GetViewMatrixRH() const = 0;
	virtual mathfu::Matrix4x4f GetViewMatrixLH() const = 0;
	virtual mathfu::Matrix4x4f GetPrevViewMatrixRH() const = 0;
	virtual mathfu::Matrix4x4f GetPrevViewMatrixLH() const = 0;
	virtual mathfu::Matrix4x4f GetProjectionMatrixRH() const = 0;
	virtual mathfu::Matrix4x4f GetProjectionMatrixLH() const = 0;

protected:
	std::string m_name;

	mathfu::Vector3f m_position;
	mathfu::Vector3f m_upVector;
	mathfu::Vector3f m_lookdir;

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
