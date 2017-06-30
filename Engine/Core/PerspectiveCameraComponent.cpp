#include "PerspectiveCameraComponent.hpp"

using namespace gxeng;

PerspectiveCameraComponent::PerspectiveCameraComponent(PerspectiveCamera* cam)
:WorldComponent(TYPE), cam(cam)
{

}

void PerspectiveCameraComponent::SetDirNormed(const Vec3& dir)
{
	assert(0); // TODO
	//cam->SetDirNormed(dir);
	//SetRot(cam->GetRot());
}

void PerspectiveCameraComponent::SetHorizontalFOV(float angleRad)
{
	cam->SetFOVAxis(angleRad, cam->GetFOVVertical());
}

void PerspectiveCameraComponent::SetVerticalFOV(float angleRad)
{
	cam->SetFOVAxis(cam->GetFOVHorizontal(), angleRad);
}

void PerspectiveCameraComponent::SetNearPlane(float val)
{
	cam->SetNearPlane(val);
}

void PerspectiveCameraComponent::SetFarPlane(float val)
{
	cam->SetFarPlane(val);
}

void PerspectiveCameraComponent::SetTarget(const Vec3& p)
{
	cam->SetTarget(mathfu::Vector3f(p.x, p.y, p.z));
}

Vec3 PerspectiveCameraComponent::GetFrontDir()
{
	assert(0); // TODO
	return Vec3();
}

Vec3 PerspectiveCameraComponent::GetBackDir()
{
	assert(0); // TODO
	return Vec3();
}

Vec3 PerspectiveCameraComponent::GetUpDir()
{
	assert(0); // TODO
	return Vec3();
}

Vec3 PerspectiveCameraComponent::GetDownDir()
{
	assert(0); // TODO
	return Vec3();
}

Vec3 PerspectiveCameraComponent::GetRightDir()
{
	assert(0); // TODO
	return Vec3();
}

Vec3 PerspectiveCameraComponent::GetLeftDir()
{
	assert(0); // TODO
	return Vec3();
}

PerspectiveCamera* PerspectiveCameraComponent::GetCam()
{
	return cam;
}