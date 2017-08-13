#pragma once
#include "Part.hpp"
#include <GraphicsEngine_LL\PerspectiveCamera.hpp>
#include <InlineMath.hpp>
#include <BaseLibrary\Common.hpp>

namespace inl::core {

using namespace inl;

class PerspCameraPart : virtual public Part
{
public:
	static const ePartType TYPE = CAMERA;

public:
	PerspCameraPart(Scene* scene, gxeng::PerspectiveCamera* cam);
	~PerspCameraPart();

	void SetViewportRect(RectF rect);
	void SetActive(bool active);

	Ray ScreenPointToRay(const Vec2& screenPoint);

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
	
	RectF viewportRect;
	bool bActive;
};

} // namespace inl::core