#pragma once

#include "../Components/GraphicsMeshComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <GameLogic/System.hpp>


namespace inl::gamelib {


class MeshTransformSystem : public game::System<MeshTransformSystem, const TransformComponent, GraphicsMeshComponent> {
public:
	void UpdateEntity(float elapsed, const TransformComponent& transform, GraphicsMeshComponent& mesh);
};


} // namespace inl::gamelib
