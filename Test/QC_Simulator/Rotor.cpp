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
	// just testing equations
	mathfu::Vector3f F;
	mathfu::Vector3f T;
	mathfu::Vector4f rpm;

	cout << "lift(3760 rpm) = " << Lift(3760.f, diameter, elevation, cdrag, airdensity) << endl;
	cout << "lift(3770 rpm) = " << Lift(3770.f, diameter, elevation, cdrag, airdensity) << endl;
	cout << "drag(7000 rpm) = " << Drag(7000.f, diameter, elevation, cdrag, airdensity) << endl << endl;

		F = { 0, 0, 19.62f };
	T = { 0, 0, 0 };
	SetTorque(F, T, rpm);
	cout << "[" << rpm[0] << ", " << rpm[1] << ", " << rpm[2] << ", " << rpm[3] << "]" << endl;
	SetRPM(rpm, F, T);
	cout << "[" << F[0] << ", " << F[1] << ", " << F[2] << "]" << endl;
	cout << "[" << T[0] << ", " << T[1] << ", " << T[2] << "]" << endl << endl;

	F = { 0, 0, 19.62f };
	T = { 0, 0, 0.1f };
	SetTorque(F, T, rpm);
	cout << "[" << rpm[0] << ", " << rpm[1] << ", " << rpm[2] << ", " << rpm[3] << "]" << endl;
	SetRPM(rpm, F, T);
	cout << "[" << F[0] << ", " << F[1] << ", " << F[2] << "]" << endl;
	cout << "[" << T[0] << ", " << T[1] << ", " << T[2] << "]" << endl << endl;

	F = { 0, 0, 19.62f };
	T = { 0.1f, 0, 0 };
	SetTorque(F, T, rpm);
	cout << "[" << rpm[0] << ", " << rpm[1] << ", " << rpm[2] << ", " << rpm[3] << "]" << endl;
	SetRPM(rpm, F, T);
	cout << "[" << F[0] << ", " << F[1] << ", " << F[2] << "]" << endl;
	cout << "[" << T[0] << ", " << T[1] << ", " << T[2] << "]" << endl << endl;

	F = { 0, 0, 19.62f };
	T = { 0, 0.1f, 0 };
	SetTorque(F, T, rpm);
	cout << "[" << rpm[0] << ", " << rpm[1] << ", " << rpm[2] << ", " << rpm[3] << "]" << endl;
	SetRPM(rpm, F, T);
	cout << "[" << F[0] << ", " << F[1] << ", " << F[2] << "]" << endl;
	cout << "[" << T[0] << ", " << T[1] << ", " << T[2] << "]" << endl << endl;
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
		torque *= 0.98f;
	} while (lifts[0] <= 0 && lifts[1] <= 0 && lifts[2] <= 0 && lifts[3] <= 0);

	// lift = cl * rpm^2 / 3600
	// rpm^2 = lift/cl*3600
	for (int i=0; i<4; ++i) {
		rpm[i] = sqrt(lifts[i] / cl * 3600);
	}
}