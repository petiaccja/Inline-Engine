#pragma once
#include "Part.hpp"
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <InlineMath.hpp>

namespace inl::core {

using namespace inl;

class DirectionalLightPart : virtual public Part
{
public:
	static const ePartType TYPE = DIRECTIONAL_LIGHT;

public:
	DirectionalLightPart(Scene* scene, gxeng::Scene* graphicsScene);
	~DirectionalLightPart();

	void UpdateEntityTransform() override;

	void SetDirection(const Vec3& direction);
	void SetColor(const Vec3& color);

protected:
	gxeng::DirectionalLight* light;
	gxeng::Scene* graphicsScene;
};

} // namespace inl::core