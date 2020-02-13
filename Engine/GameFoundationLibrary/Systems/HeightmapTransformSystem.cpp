#include "HeightmapTransformSystem.hpp"

namespace inl::gamelib {

void HeightmapTransformSystem::UpdateEntity(float elapsed, const TransformComponent& transform, GraphicsHeightmapComponent& heightmap) {
	heightmap.entity->Transform() = transform;
}

} // namespace inl::gamelib