#include "PerspCameraPart.hpp"
#include "Scene.hpp"

namespace inl::core {

PerspCameraPart::PerspCameraPart(Scene* scene, gxeng::PerspectiveCamera* cam)
:Part(scene, TYPE), cam(cam), aspectRatio(1.f), bActive(false)
{
	cam->SetTargeted(true);

	SetPos(cam->GetPosition());
	SetTarget(cam->GetTarget());
}

PerspCameraPart::~PerspCameraPart()
{
	delete cam;
}

void PerspCameraPart::SetViewportRect(RectF rect)
{
	viewportRect = rect;
}

void PerspCameraPart::SetActive(bool bActive)
{
	this->bActive = bActive;
}

void PerspCameraPart::UpdateEntityTransform()
{
	// Update camera Entity position
	cam->SetPosition(GetPos());

	// Update camera Entity Rotation
	cam->SetUpVector(GetUpDir());
	cam->SetTarget(GetTarget());
}

Ray PerspCameraPart::ScreenPointToRay(const Vec2& screenPoint)
{
	Ray result;
	
	float nearPlaneHalfWidth = abs(tan(cam->GetFOVHorizontal() * 0.5)) * GetNearPlaneDist();
	float nearPlaneHalfHeight = abs(tan(cam->GetFOVVertical() * 0.5)) * GetNearPlaneDist();

	float x = screenPoint.x / viewportRect.GetWidth();
	float y = screenPoint.y / viewportRect.GetHeight();

	// Make x and y relative to screen center
	x = x * 2 - 1;
	y = y * -2 + 1;

	// Convert x and y to world space
	x *= nearPlaneHalfWidth;
	y *= nearPlaneHalfHeight;

	Vec3 pointOnNearPlane = GetPos() + GetFrontDir() * GetNearPlaneDist() + GetRightDir() * x + GetUpDir() * y;

	Vec3 pos = GetPos();
	result.origin = pointOnNearPlane;
	result.direction = (pointOnNearPlane - GetPos()).Normalized();
	return result;
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