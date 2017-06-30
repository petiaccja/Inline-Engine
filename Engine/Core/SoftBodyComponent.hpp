#pragma once
#include "WorldComponent.hpp"

using namespace inl;

namespace inl::physics { class ISoftBodyEntity; }

namespace inl::core {

class SoftBodyComponent : public WorldComponent
{
public:
	static const eWorldComponentType TYPE = SOFT_BODY;

public:
	SoftBodyComponent(physics::ISoftBodyEntity* e);

	physics::ISoftBodyEntity* GetEntity();

protected:
	physics::ISoftBodyEntity* entity;
};

}