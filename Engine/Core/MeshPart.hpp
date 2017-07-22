#pragma once
#include "Part.hpp"
#include <GraphicsEngine_LL\MeshEntity.hpp>

namespace inl::core {

class MeshPart : virtual public Part
{
public:
	static const ePartType TYPE = MESH;

public:
	MeshPart(gxeng::MeshEntity* e);

	void UpdateEntityTransform() override;

protected:
	gxeng::MeshEntity* entity;
};

} // namespace inl::core