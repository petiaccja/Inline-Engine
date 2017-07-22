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

	void SetDirNormed(const Vec3& dir);

	void SetAspectRatio(float ratio);
	void SetFOV(float fov);

	void SetNearPlane(float val);
	void SetFarPlane(float val);
	void SetTarget(const Vec3& p);

	Vec3 GetFrontDir();
	Vec3 GetBackDir();
	Vec3 GetUpDir();
	Vec3 GetDownDir();
	Vec3 GetRightDir();
	Vec3 GetLeftDir();

protected:
	gxeng::PerspectiveCamera* cam;

	float aspectRatio;
};

} // namespace inl::core