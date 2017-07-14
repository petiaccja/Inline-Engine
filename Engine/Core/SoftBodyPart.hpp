#pragma once
#include "Part.hpp"

namespace inl::physics { class ISoftBodyEntity; }

namespace inl::core {

using namespace inl::physics;


class SoftBodyPart : public Part
{
public:
	static const ePartType TYPE = SOFT_BODY;

public:
	SoftBodyPart(ISoftBodyEntity* e);

	ISoftBodyEntity* GetEntity();

protected:
	ISoftBodyEntity* entity;
};

}