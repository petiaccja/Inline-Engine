#pragma once

#include <GraphicsEngine/Scene/IBasicCamera.hpp>

#include <InlineMath.hpp>
#include <string>


namespace inl::gxeng {

/// <summary>
/// Abstract base class of perspective and orthographic cameras
/// </summary>
class BasicCamera : virtual public IBasicCamera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};

public:
	BasicCamera();
	virtual ~BasicCamera() = default;

	void SetName(std::string name) override;
	const std::string& GetName() const override;

	// Set positional properties.
	void SetPosition(Vec3 position) override;
	void SetTarget(Vec3 targetPosition) override;
	void SetLookDirection(Vec3 lookDirection) override;
	void SetUpVector(Vec3 upVector) override;
	void SetTargeted(bool targeted) override;

	// Set rendering properties.
	void SetFocus(float focusDistance) override;

	// negative values are valid
	void SetNearPlane(float zOffset) override;
	void SetFarPlane(float zOffset) override;

	// Get positional properties.
	Vec3 GetPosition() const override;
	Vec3 GetLookDirection() const override;
	Vec3 GetTarget() const override;
	float GetTargetDistance() const override;
	Vec3 GetUpVector() const override;

	// Get rendering properties.
	float GetFocus() const override;

	// Get depth plane Z offset.
	float GetNearPlane() const override;
	float GetFarPlane() const override;

	float GetAspectRatio() const override = 0;

	Mat44 GetViewMatrix() const override = 0;
	Mat44 GetProjectionMatrix() const override = 0;
	Mat44 GetPrevViewMatrix() const override = 0;

protected:
	std::string m_name;

	Vec3 m_position;
	Vec3 m_upVector;
	Vec3 m_lookdir;
	Vec3 m_prevPosition;
	Vec3 m_prevUpVector;
	Vec3 m_prevLookdir;

	float m_targetDistance;
	bool m_targeted = false;

	float m_focus;

	float m_nearPlane;
	float m_farPlane;
};


} // namespace inl::gxeng
