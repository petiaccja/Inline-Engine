#include "MeshComponent.hpp"
#include "RigidBodyComponent.hpp"
#include "SoftBodyComponent.hpp"
#include "PerspectiveCameraComponent.hpp"
#include "GameWorld.hpp"

float Radians(float deg)
{
	return deg / 180 * 3.14159265358979;
}

PerspectiveCameraComponent* WorldComponent::AddComponent_Camera()
{
	PerspectiveCameraComponent* c = World.AddComponent_Camera();
	Attach(c);

	return c;
}

MeshComponent* WorldComponent::AddComponent_Mesh(const std::string& modelPath)
{
	MeshComponent* c = World.AddComponent_Mesh(modelPath);
	Attach(c);

	return c;
}