#pragma once
#include "Part.hpp"
#include <GraphicsEngine_LL\PerspectiveCamera.hpp>
#include <InlineMath.hpp>

namespace inl::core {

using namespace inl;

class PerspCameraPart : virtual public Part
{
public:
	static const ePartType TYPE = CAMERA;

public:
	PerspCameraPart(gxeng::PerspectiveCamera* cam);
	~PerspCameraPart();

	void UpdateEntityTransform() override;

	void SetDir(const Vec3& dir);

	void SetAspectRatio(float ratio);
	void SetFOV(float fov);

	void SetNearPlaneDist(float val);
	void SetFarPlaneDist(float val);
	void SetTarget(const Vec3& p);

	float GetNearPlaneDist();
	float GetFarPlaneDist();

	Vec3 GetTarget();

	Vec3 GetFrontDir();
	Vec3 GetBackDir();
	Vec3 GetUpDir();
	Vec3 GetDownDir();
	Vec3 GetRightDir();
	Vec3 GetLeftDir();

protected:
	gxeng::PerspectiveCamera* cam;

	float aspectRatio;
	Vec3 target;
};

} // namespace inl::core