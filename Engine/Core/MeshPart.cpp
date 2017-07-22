#include "MeshPart.hpp"
#include "Core.hpp"

namespace inl::core {

MeshPart::MeshPart(gxeng::MeshEntity* e)
: Part(TYPE), entity(e)
{
}

void MeshPart::UpdateEntityTransform()
{

}

} // namespace inl::core