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
	const Quat& rot = GetRot();
	Vec3 upVec = rot * Vec3(0, 0, 1);
	Vec3 frontVec = rot * Vec3(0, 1, 0);

	// Update camera Entity position
	cam->SetPosition(GetPos());
	
	// Update camera Entity Rotation
	cam->SetUpVector(upVec);
	cam->SetTarget(cam->GetPosition() + frontVec);
}

void PerspCameraPart::SetDirNormed(const Vec3& dir)
{
	// TODO do not change the distance between camera position and camera target, save the length of it and reuse it !
	//cam->SetTarget(cam->GetTarget() + mathfu::Vector3f(dir.x, dir.y, dir.z));

	//assert(0);

	//mathfu::Vector3f frontDir = cam->GetLookDirection();
	//mathfu::Vector3f upVec = cam->GetUpVector();
	//mathfu::Vector3f rightVec = mathfu::Vector3f::CrossProduct(frontDir, upVec).Normalized();
	//
	//// TODO generate camera rotation from {front, up, right} dirs
	//Mat33 mat;
	//mat(0, 0) = rightVec.x();
	//mat(0, 1) = rightVec.y();
	//mat(0, 2) = rightVec.z();
	//
	//mat(1, 0) = upVec.x();
	//mat(1, 1) = upVec.y();
	//mat(1, 2) = upVec.z();
	//
	//mat(2, 0) = frontDir.x();
	//mat(2, 1) = frontDir.y();
	//mat(2, 2) = frontDir.z();
	//
	//Quat camRot = (Quat)mat;
	//
	//SetRot(camRot);
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
	SetDirNormed(target - GetPos());
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