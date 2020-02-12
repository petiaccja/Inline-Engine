#pragma once

#include "ActionHeap.hpp"
#include "ActionSystem.hpp"

#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameLogic/System.hpp>


class CameraMoveSystem : public inl::game::System<CameraMoveSystem, inl::gamelib::PerspectiveCameraComponent>, public ActionSystem {
public:
	void ReactActions(ActionHeap& actions) override;
	void UpdateEntity(float elapsed, inl::gamelib::PerspectiveCameraComponent& camera);
	void EmitActions(ActionHeap& actions) override;

private:
	std::optional<std::reference_wrapper<ActionHeap>> m_transientActionHeap;
};
