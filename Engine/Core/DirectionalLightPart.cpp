#include "DirectionalLightPart.hpp"

namespace inl::core {

DirectionalLightPart::DirectionalLightPart(gxeng::Scene* scene)
:Part(TYPE), scene(scene)
{
	light = new gxeng::DirectionalLight();
	scene->GetDirectionalLights().Add(light);
}

DirectionalLightPart::~DirectionalLightPart()
{
	scene->GetDirectionalLights().Remove(light);
	delete light;
}

void DirectionalLightPart::UpdateEntityTransform()
{

}

void DirectionalLightPart::SetDirection(const Vec3& direction)
{
	light->SetDirection(mathfu::Vector3f(direction.x, direction.y, direction.z));
}

void DirectionalLightPart::SetColor(const Vec3& color)
{
	light->SetColor(mathfu::Vector3f(color.x, color.y, color.z));
}

} // namespace inl::core