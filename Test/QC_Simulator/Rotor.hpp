#pragma once

#include <mathfu/mathfu_exc.hpp>
#include <InlineMath.hpp>


class Rotor {
public:
	Rotor();
	void SetRPM(inl::Vec4 rpm, inl::Vec3& force, inl::Vec3& torque) const;
	void SetTorque(inl::Vec3 force, inl::Vec3 torque, inl::Vec4& rpm) const;
private:
	float diameter = .30f, elevation = .12f, cdrag = 1.0f, clift = 1.0f;
	float airdensity = 1.225f;
	float arm = 0.24f;
};