#include "Rotor.hpp"


#include <cmath>
#include <iostream>
using std::cout;
using std::endl;


static constexpr float pi = 3.1415926f;



float LiftCoeff(float diameter, float elevation, float clift, float airdensity) {
	return clift * airdensity * pow(elevation, 2) * pi * pow(diameter / 2, 2);
}

float DragCoeff(float diameter, float elevation, float cdrag, float airdensity) {
	return cdrag * airdensity * 0.37 * pow(diameter / 2, 5);
}

float Lift(float rpm, float diam, float elev, float c, float airdensity) {
	//float v = rpm / 60 * elev; // relative air velocity
	//float A = diam*diam / 4 * pi; // rotor disk area
	//float d = 1.225f; // air density
	//return c*d*A*v*v;

	return LiftCoeff(diam, elev, c, airdensity)*rpm*rpm / 3600.f;
}

float Drag(float rpm, float diam, float elev, float c, float airdensity) {
	//float d = 1.225f;
	//return 0.37*c*d*pow(diam / 2, 5.0f)*pow(rpm / 60, 2);

	return DragCoeff(diam, elev, c, airdensity)*rpm*rpm / 3600.f;
}


Rotor::Rotor() {

}


void Rotor::SetRPM(mathfu::Vector4f rpm, mathfu::Vector3f& force, mathfu::Vector3f& torque) const {
	mathfu::Vector4f lifts = {
		Lift(rpm[0], diameter, elevation, clift, airdensity),
		Lift(rpm[1], diameter, elevation, clift, airdensity),
		Lift(rpm[2], diameter, elevation, clift, airdensity),
		Lift(rpm[3], diameter, elevation, clift, airdensity)
	};
	mathfu::Vector4f drags = {
		Drag(rpm[0], diameter, elevation, cdrag, airdensity),
		Drag(rpm[1], diameter, elevation, cdrag, airdensity),
		Drag(rpm[2], diameter, elevation, cdrag, airdensity),
		Drag(rpm[3], diameter, elevation, cdrag, airdensity)
	};

	force = { 0, 0, lifts[0] + lifts[1] + lifts[2] + lifts[3] };

	float Tx = arm/sqrt(2) * ((lifts[0] + lifts[1]) - (lifts[2] + lifts[3]));
	float Ty = arm/sqrt(2) * ((lifts[0] + lifts[2]) - (lifts[1] + lifts[3]));
	float Tz = drags[0] - drags[1] + drags[3] - drags[2];

	torque = { Tx, Ty, Tz };
}


void Rotor::SetTorque(mathfu::Vector3f force, mathfu::Vector3f torque, mathfu::Vector4f& rpm) const {
	float cl = LiftCoeff(diameter, elevation, cdrag, airdensity);
	float cd = DragCoeff(diameter, elevation, cdrag, airdensity);
	float c = cd / cl;
	float a = arm/sqrt(2);

	mathfu::Vector4f lifts;
	do {
		lifts[0] = (c*(force.z() + torque.x() / a + torque.y() / a) + torque.z()) / (4 * c);
		lifts[1] = (c*(force.z() + torque.x() / a - torque.y() / a) - torque.z()) / (4 * c);
		lifts[2] = (c*(force.z() - torque.x() / a + torque.y() / a) - torque.z()) / (4 * c);
		lifts[3] = (c*(force.z() - torque.x() / a - torque.y() / a) + torque.z()) / (4 * c);

		// lift = cl * rpm^2 / 3600
		// rpm^2 = lift/cl*3600
		for (int i = 0; i<4; ++i) {
			rpm[i] = sqrt(lifts[i] / cl * 3600);
		}

		torque *= 0.98f;
	} while (lifts[0] <= 0 || lifts[1] <= 0 || lifts[2] <= 0 || lifts[3] <= 0
		|| rpm[0] <= 1000 || rpm[1] <= 1000 || rpm[2] <= 1000 || rpm[3] <= 1000);

	if (isnan(rpm.x()) || isnan(rpm.y()) || isnan(rpm.z()) || isnan(rpm.w())) {
		__debugbreak();
	}
}