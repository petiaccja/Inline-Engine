#pragma once

#include <mathfu/mathfu_exc.hpp>


class Rotor {
public:
	void SetRPM(mathfu::Vector4f rpm);
	mathfu::Vector3f NetForce();
	mathfu::Vector3f NetTorque();
private:
	mathfu::Vector4f rpm{ 0,0,0,0 };
};