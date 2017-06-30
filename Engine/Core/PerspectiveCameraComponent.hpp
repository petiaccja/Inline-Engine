#pragma once

#include "WorldComponent.hpp"
#include <GraphicsEngine_LL\PerspectiveCamera.hpp>

class PerspectiveCameraComponent : public WorldComponent
{
public:
	static const eWorldComponentType TYPE = CAMERA;

public:
	PerspectiveCameraComponent(gxeng::PerspectiveCamera* cam);

	void SetDirNormed(const Vec3& dir);

	void SetHorizontalFOV(float angleRad);
	void SetVerticalFOV(float angleRad);
	void SetNearPlane(float val);
	void SetFarPlane(float val);
	void SetTarget(const Vec3& p);

	Vec3 GetFrontDir();
	Vec3 GetBackDir();
	Vec3 GetUpDir();
	Vec3 GetDownDir();
	Vec3 GetRightDir();
	Vec3 GetLeftDir();

	gxeng::PerspectiveCamera* GetCam();

protected:
	gxeng::PerspectiveCamera* cam;
};