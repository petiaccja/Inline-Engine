#include "DirectionalLightPart.hpp"

namespace inl::core {

DirectionalLightPart::DirectionalLightPart(Scene* scene, gxeng::Scene* graphicsScene)
:Part(scene, TYPE), graphicsScene(graphicsScene)
{
	light = new gxeng::DirectionalLight();
	graphicsScene->GetEntities<gxeng::DirectionalLight>().Add(light);
}

DirectionalLightPart::~DirectionalLightPart()
{
	graphicsScene->GetEntities<gxeng::DirectionalLight>().Remove(light);
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