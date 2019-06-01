#pragma once

#include <InlineMath.hpp>


namespace inl::gxeng {


/// <summary>
/// Abstract base class of perspective and orthographic cameras
/// </summary>
class IBasicCamera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};

public:
	virtual ~IBasicCamera() = default;

	virtual void SetName(std::string name) = 0;
	virtual const std::string& GetName() const = 0;

	// Set positional properties.
	virtual void SetPosition(Vec3 position) = 0;
	virtual void SetTarget(Vec3 targetPosition) = 0;
	virtual void SetLookDirection(Vec3 lookDirection) = 0;
	virtual void SetUpVector(Vec3 upVector) = 0;
	virtual void SetTargeted(bool targeted) = 0;

	// Set rendering properties.
	virtual void SetFocus(float focusDistance) = 0;

	// negative values are valid
	virtual void SetNearPlane(float zOffset) = 0;
	virtual void SetFarPlane(float zOffset) = 0;

	// Get positional properties.
	virtual Vec3 GetPosition() const = 0;
	virtual Vec3 GetLookDirection() const = 0;
	virtual Vec3 GetTarget() const = 0;
	virtual float GetTargetDistance() const = 0;
	virtual Vec3 GetUpVector() const = 0;

	// Get rendering properties.
	virtual float GetFocus() const = 0;

	// Get depth plane Z offset.
	virtual float GetNearPlane() const = 0;
	virtual float GetFarPlane() const = 0;

	virtual float GetAspectRatio() const = 0;

	virtual Mat44 GetViewMatrix() const = 0;
	virtual Mat44 GetProjectionMatrix() const = 0;
	virtual Mat44 GetPrevViewMatrix() const = 0;
};


} // namespace inl::gxeng