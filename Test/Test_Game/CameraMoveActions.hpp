#pragma once

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
