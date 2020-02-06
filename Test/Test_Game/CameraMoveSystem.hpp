#pragma once

#include "ActionHeap.hpp"

#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameLogic/System.hpp>


class CameraMoveSystem : public inl::game::SpecificSystem<CameraMoveSystem, inl::gamelib::PerspectiveCameraComponent> {
public:
	explicit CameraMoveSystem(std::shared_ptr<ActionHeap> actionHeap);

	void UpdateEntity(float elapsed, inl::gamelib::PerspectiveCameraComponent& camera);

private:
	std::shared_ptr<ActionHeap> m_actionHeap;
};