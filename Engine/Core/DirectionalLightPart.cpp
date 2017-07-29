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
	light->SetDirection(direction);
}

void DirectionalLightPart::SetColor(const Vec3& color)
{
	light->SetColor(color);
}

} // namespace inl::core