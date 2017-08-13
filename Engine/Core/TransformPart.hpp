#pragma once
#include "Part.hpp"
#include <GraphicsEngine_LL\MeshEntity.hpp>

namespace inl::core {

class TransformPart : virtual public Part
{
public:
	static const ePartType TYPE = TRANSFORM;

public:
	TransformPart(Scene* scene);

	void UpdateEntityTransform() override;
};

} // namespace inl::core