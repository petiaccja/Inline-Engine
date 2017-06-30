#pragma once
#include "WorldComponent.hpp"

class Transform3DComponent : public WorldComponent
{
public:
	static const eWorldComponentType TYPE = TRANSFORM;

public:
	inline Transform3DComponent();
};

Transform3DComponent::Transform3DComponent()
:WorldComponent(TYPE)
{

}