#include "MeshPart.hpp"
#include "RigidBodyPart.hpp"
#include "SoftBodyPart.hpp"
#include "PerspCameraPart.hpp"
#include "SceneScript.hpp"
#include "Scene.hpp"

using namespace inl::core;

// TEMPORARY !!
float Radians(float deg)
{
	return deg / 180 * 3.14159265358979;
}

Part::Part(ePartType type)
:parent(0), bDirtyRelativeTransform(false), bDirtyTransform(false), type(type), bKilled(false)
{
}

Part::~Part()
{

}

void Part::SetScale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot)
{
	Vec3 dScale = scale / transform.GetScale();

	Scale(dScale, rootPos, rootRot);
}

void Part::Scale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot)
{
	std::function<void(Part* child)> func;
	func = [&](Part* child)
	{
		child->transform.Scale(scale, rootPos, rootRot);

		for (Part* c : child->children)
			func(c);
	};

	// Scale hierarchy
	func(this);
}

ePartType Part::GetType()
{
	return type;
}

PerspCameraPart* Part::AddPart_Camera()
{
	PerspCameraPart* c = scene->AddPart_Camera();
	Attach(c);

	return c;
}

MeshPart* Part::AddPart_Mesh(const std::string& modelPath)
{
	MeshPart* c = scene->AddPart_Mesh(modelPath);
	Attach(c);

	return c;
}