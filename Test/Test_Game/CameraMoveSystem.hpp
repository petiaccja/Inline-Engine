#pragma once

#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameLogic/System.hpp>
#include "ActionHeap.hpp"


enum class eCameraMoveAxis {
	FORWARD,
	RIGHT,
	UP,
};

enum class eCameraRotationAxis {
	PITCH,
	YAW,
};

struct CameraMoveAction {
	float speed = 0.0f; // Unit/sec
	eCameraMoveAxis direction = eCameraMoveAxis::FORWARD;
};

struct CameraRotateAction {
	float speed = 0.0f; // Radian/sec
	eCameraRotationAxis direction = eCameraRotationAxis::PITCH;
};


class CameraMoveSystem : public inl::game::SpecificSystem<CameraMoveSystem, inl::gamelib::PerspectiveCameraComponent> {
public:
	explicit CameraMoveSystem(std::shared_ptr<ActionHeap> actionHeap);
	
	void UpdateEntity(float elapsed, inl::gamelib::PerspectiveCameraComponent& camera);
	
private:
	std::shared_ptr<ActionHeap> m_actionHeap;
};