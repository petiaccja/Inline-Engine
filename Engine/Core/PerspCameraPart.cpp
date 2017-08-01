#include "PerspCameraPart.hpp"

namespace inl::core {

PerspCameraPart::PerspCameraPart(gxeng::PerspectiveCamera* cam)
:Part(TYPE), cam(cam), aspectRatio(1.f)
{
	cam->SetTargeted(true);

	SetPos(cam->GetPosition());
	SetTarget(cam->GetTarget());
}

PerspCameraPart::~PerspCameraPart()
{
	delete cam;
}

void PerspCameraPart::UpdateEntityTransform()
{
	// Update camera Entity position
	cam->SetPosition(GetPos());

	// Update camera Entity Rotation
	cam->SetUpVector(GetUpDir());
	cam->SetTarget(GetTarget());
}

void PerspCameraPart::SetDir(const Vec3& dir)
{
	Vec3 frontDir = dir.Normalized();
	Vec3 upDir(0, 0, 1);
	Vec3 rightDir = Cross(frontDir, upDir).Normalized();
	upDir = Cross(rightDir, frontDir);

	Mat33 mat;
	mat(0, 0) = rightDir.x;
	mat(0, 1) = rightDir.y;
	mat(0, 2) = rightDir.z;

	mat(1, 0) = frontDir.x;
	mat(1, 1) = frontDir.y;
	mat(1, 2) = frontDir.z;

	mat(2, 0) = upDir.x;
	mat(2, 1) = upDir.y;
	mat(2, 2) = upDir.z;

	SetRot((Quat)mat);
}

void PerspCameraPart::SetAspectRatio(float ratio)
{
	aspectRatio = ratio;
	cam->SetFOVAxis(cam->GetFOVVertical() * ratio, cam->GetFOVVertical());
}

void PerspCameraPart::SetFOV(float fov)
{
	cam->SetFOVAxis(fov * aspectRatio, fov);
}

void PerspCameraPart::SetNearPlaneDist(float val)
{
	cam->SetNearPlane(val);
}

void PerspCameraPart::SetFarPlaneDist(float val)
{
	cam->SetFarPlane(val);
}

void PerspCameraPart::SetTarget(const Vec3& p)
{
	target = p;

	// Update rotation
	SetDir(target - GetPos());
}

Vec3 PerspCameraPart::GetTarget()
{
	return target;
	//mathfu::Vector3f target = cam->GetTarget();
	//
	//return Vec3(target.x(), target.y(), target.z());
}

float PerspCameraPart::GetNearPlaneDist()
{
	return cam->GetNearPlane();
}

float PerspCameraPart::GetFarPlaneDist()
{
	return cam->GetFarPlane();
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

} // namespace inl::core