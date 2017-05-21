#pragma once

#include <mathfu/mathfu_exc.hpp>
#include <InlineMath.hpp>


class PIDController {
public:
	PIDController();
	void Update(mathfu::Quaternionf r, float lift, mathfu::Quaternionf q, inl::Vec3 w, float elapsed, inl::Vec3& force, inl::Vec3& torque);
	void SetInertia(inl::Mat33 inertia) { this->inertia = inertia; }
private:
	inl::Mat33 inertia;

	mathfu::Quaternionf e; // P
	mathfu::Quaternionf ie; // I
	inl::Vec3 de; // D

	inl::Vec3 Kp, Ki, Kd;
};