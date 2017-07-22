#include "PerspCameraPart.hpp"

namespace inl::core {

PerspCameraPart::PerspCameraPart(gxeng::PerspectiveCamera* cam)
:Part(TYPE), cam(cam), aspectRatio(1.f)
{
	cam->SetTargeted(true);
}

PerspCameraPart::~PerspCameraPart()
{
	delete cam;
}

void PerspCameraPart::UpdateEntityTransform()
{
	auto pos = GetPos();
	auto rot = GetRot();
	Vec3 upVec = rot * Vec3(0, 0, 1);
	Vec3 frontVec = rot * Vec3(0, 1, 0);

	// Update camera Entity position
	cam->SetPosition(mathfu::Vector3f(pos.x, pos.y, pos.z));
	
	// Update camera Entity Rotation
	cam->SetUpVector(mathfu::Vector3f(upVec.x, upVec.y, upVec.z));
	cam->SetTarget(cam->GetPosition() + mathfu::Vector3f(frontVec.x, frontVec.y, frontVec.z));
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

void PerspCameraPart::SetAspectRatio(float ratio)
{
	aspectRatio = ratio;
	cam->SetFOVAxis(cam->GetFOVVertical() * ratio, cam->GetFOVVertical());
}

void PerspCameraPart::SetFOV(float fov)
{
	cam->SetFOVAxis(fov * aspectRatio, fov);
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

} // namespace inl::core