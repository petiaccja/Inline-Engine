#pragma once

#include <InlineMath.hpp>


class PIDController {
public:
	PIDController();
	void Update(inl::Quat r, float lift, inl::Quat q, inl::Vec3 w, float elapsed, inl::Vec3& force, inl::Vec3& torque);
	void SetInertia(inl::Mat33 inertia) { this->inertia = inertia; }
private:
	inl::Mat33 inertia;

	inl::Quat e; // P
	inl::Quat ie; // I
	inl::Vec3 de; // D

	inl::Vec3 Kp, Ki, Kd;
};