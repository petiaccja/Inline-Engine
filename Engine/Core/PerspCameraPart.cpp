#include "PerspCameraPart.hpp"

namespace inl::core {

PerspCameraPart::PerspCameraPart(gxeng::PerspectiveCamera* cam)
:Part(TYPE), cam(cam)
{

}

void PerspCameraPart::SetDirNormed(const Vec3& dir)
{
	// TODO do not change the distance between camera position and camera target, save the length of it and reuse it !
	cam->SetTarget(cam->GetTarget() + mathfu::Vector3f(dir.x, dir.y, dir.z));

	mathfu::Vector3f frontDir = cam->GetLookDirection();
	mathfu::Vector3f upVec = cam->GetUpVector();
	mathfu::Vector3f rightVec = mathfu::Vector3f::CrossProduct(frontDir, upVec).Normalized();

	// TODO generate camera rotation from front up and right dirs
	Quat camRot;
	assert(0);

	SetRot(camRot);
}

void PerspCameraPart::SetHorizontalFOV(float angleRad)
{
	cam->SetFOVAxis(angleRad, cam->GetFOVVertical());
}

void PerspCameraPart::SetVerticalFOV(float angleRad)
{
	cam->SetFOVAxis(cam->GetFOVHorizontal(), angleRad);
}

void PerspCameraPart::SetNearPlane(float val)
{
	cam->SetNearPlane(val);
}

void PerspCameraPart::SetFarPlane(float val)
{
	cam->SetFarPlane(val);
}

void PerspCameraPart::SetTarget(const Vec3& p)
{
	cam->SetTarget(mathfu::Vector3f(p.x, p.y, p.z));
}

Vec3 PerspCameraPart::GetFrontDir()
{
	return GetRot() * Vec3(0, 1, 0);
}

Vec3 PerspCameraPart::GetBackDir()
{
	return GetRot() * Vec3(0, -1, 0);
}

Vec3 PerspCameraPart::GetUpDir()
{
	return GetRot() * Vec3(0, 0, 1);
}

Vec3 PerspCameraPart::GetDownDir()
{
	return GetRot() * Vec3(0, 0, -1);
}

Vec3 PerspCameraPart::GetRightDir()
{
	return GetRot() * Vec3(1, 0, 0);
}

Vec3 PerspCameraPart::GetLeftDir()
{
	return GetRot() * Vec3(-1, 0, 0);
}

gxeng::PerspectiveCamera* PerspCameraPart::GetCam()
{
	return cam;
}

} // namespace inl::core