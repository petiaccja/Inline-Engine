#include "MeshPart.hpp"
#include "Core.hpp"

namespace inl::core {

MeshPart::MeshPart(Scene* scene, gxeng::MeshEntity* e)
:Part(scene, TYPE), entity(e)
{
}

void MeshPart::UpdateEntityTransform()
{
	entity->SetPosition(GetPos());
	entity->SetRotation(GetRot());
	entity->SetScale(GetScale());
}

} // namespace inl::core