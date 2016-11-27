#pragma once

#include <mathfu/mathfu_exc.hpp>


class Rotor {
public:
	Rotor();
	void SetRPM(mathfu::Vector4f rpm, mathfu::Vector3f& force, mathfu::Vector3f& torque) const;
	void SetTorque(mathfu::Vector3f force, mathfu::Vector3f torque, mathfu::Vector4f& rpm) const;
private:
	float diameter = .30f, elevation = .12f, cdrag = 1.0f, clift = 1.0f;
	float airdensity = 1.225f;
	float arm = 0.25f;
};