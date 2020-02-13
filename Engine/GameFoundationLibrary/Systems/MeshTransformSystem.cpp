#include "MeshTransformSystem.hpp"


namespace inl::gamelib {

void MeshTransformSystem::UpdateEntity(float elapsed, const TransformComponent& transform, GraphicsMeshComponent& mesh) {
	mesh.entity->Transform() = transform;
}

} // namespace inl::gamelib
