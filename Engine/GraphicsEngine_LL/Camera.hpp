#pragma once


#include <string>
#include <mathfu/mathfu_exc.hpp>


namespace inl::gxeng {


class Camera {
public:
	enum eMoveBehaviour {
		KEEP_TARGET,
		KEEP_LOOKDIR,
	};
public:
	Camera();
	virtual ~Camera() {}

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
	void SetFOVAspect(float horizontalFov, float aspectRatio);
	void SetFOVAxis(float horizontalFov, float verticalFov);

	// Set depth plane Z offset.
	void SetNearPlane(float zOffset);
	void SetFarPlane(float zOffset);

	// Get positional properties.
	mathfu::Vector3f GetPosition() const;
	mathfu::Vector3f GetLookDirection() const;
	mathfu::Vector3f GetTarget() const;
	float GetTargetDistance() const;
	mathfu::Vector3f GetUpVector() const;

	// Get rendering properties.
	float GetFOVVertical() const;
	float GetFOVHorizontal() const;
	float GetAspectRatio() const;
	float GetFocus() const;

	// Get depth plane Z offset.
	float GetNearPlane() const;
	float GetFarPlane() const;

	// Matrices
	mathfu::Matrix4x4f GetViewMatrixRH() const;
	mathfu::Matrix4x4f GetViewMatrixLH() const;
	mathfu::Matrix4x4f GetPerspectiveMatrixRH() const;
	mathfu::Matrix4x4f GetPerspectiveMatrixLH() const;
	mathfu::Matrix4x4f GetOrthographicMatrixRH() const;
	mathfu::Matrix4x4f GetOrthographicMatrixLH() const;
private:
	mathfu::Vector3f m_position;
	mathfu::Vector3f m_upVector;
	mathfu::Vector3f m_lookdir;
	float m_targetDistance;
	bool m_targeted = false;

	float m_fovH;
	float m_fovV;
	float m_focus;

	float m_nearPlane;
	float m_farPlane;

	std::string m_name;
};


} // namespace inl::gxeng
