#pragma once

#include <mathfu/mathfu_exc.hpp>


class PIDController {
public:
	PIDController();
	void Update(mathfu::Quaternionf r, float lift, mathfu::Quaternionf q, mathfu::Vector3f w, float elapsed, mathfu::Vector3f& force, mathfu::Vector3f& torque);
	void SetInertia(mathfu::Matrix3x3f inertia) { this->inertia = inertia; }
private:
	mathfu::Matrix3x3f inertia;

	mathfu::Quaternionf e; // P
	mathfu::Quaternionf ie; // I
	mathfu::Vector3f de; // D

	mathfu::Vector3f Kp, Ki, Kd;
};