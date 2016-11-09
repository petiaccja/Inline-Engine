#include "Rotor.hpp"


#include <cmath>

static constexpr float pi = 3.1415926f;


float Lift(float rpm, float diam, float elev, float c) {
	float w = rpm / 60;
	float airspeed = w * elev;
	float area = pow((diam / 2), 2) * pi;
	float volume = airspeed * area;
	float mass = volume * 1.225;
	return mass*airspeed * c;
}

float Drag(float rpm, float diam, float elev, float c) {
	float w = rpm / 60;
	float R = diam / 2;
	float C = c*1.225*R / 15;
	float F = 6.6*C*pow(R, 3) * pow(w, 2);
	return 0.84*R*F;
}


void Rotor::SetRPM(mathfu::Vector4f rpm) {
	this->rpm = rpm;
}

mathfu::Vector3f Rotor::NetForce() {
	mathfu::Vector4f lifts = {
		Lift(rpm[0], .30f, .12f, 1.f),
		Lift(rpm[1], .30f, .12f, 1.f),
		Lift(rpm[2], .30f, .12f, 1.f),
		Lift(rpm[3], .30f, .12f, 1.f)
	};
	return{ 0, 0, lifts[0] + lifts[1] + lifts[2] + lifts[3] };
}

mathfu::Vector3f Rotor::NetTorque() {
	mathfu::Vector4f lifts = {
		Lift(rpm[0], .30f, .12f, 1.f),
		Lift(rpm[1], .30f, .12f, 1.f),
		Lift(rpm[2], .30f, .12f, 1.f),
		Lift(rpm[3], .30f, .12f, 1.f)
	};
	mathfu::Vector4f drags = {
		Drag(rpm[0], .30f, .12f, 1.f),
		Drag(rpm[1], .30f, .12f, 1.f),
		Drag(rpm[2], .30f, .12f, 1.f),
		Drag(rpm[3], .30f, .12f, 1.f)
	};

	float arm = 0.25f;
	float Tx = arm / sqrt(2) * ((lifts[0] + lifts[1]) - (lifts[2] + lifts[3]));
	float Ty = arm / sqrt(2) * ((lifts[0] + lifts[2]) - (lifts[1] + lifts[3]));
	float Tz = drags[0] - drags[1] + drags[3] - drags[2];

	return{ Tx, Ty, Tz };
}