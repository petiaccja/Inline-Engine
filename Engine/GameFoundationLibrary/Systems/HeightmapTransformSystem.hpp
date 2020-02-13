#pragma once

#include "../Components/GraphicsHeightmapComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <GameLogic/System.hpp>


namespace inl::gamelib {


class HeightmapTransformSystem : public game::System<HeightmapTransformSystem, const TransformComponent, GraphicsHeightmapComponent> {
public:
	void UpdateEntity(float elapsed, const TransformComponent& transform, GraphicsHeightmapComponent& heightmap);
};


} // namespace inl::gamelib
