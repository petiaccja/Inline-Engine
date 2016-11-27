#include "PIDController.hpp"
#include <fstream>

using namespace std;
using namespace mathfu;


static ofstream file("control.txt");


PIDController::PIDController() {
	e = { 1, 0, 0, 0 };
	ie = { 1, 0, 0, 0 };
	de = { 0, 0, 0 };

	Kp = 2.0f;
	Ki = 0.3f;
	Kd = 0.3f;

	//Ki = 0;
	//Kd = 0;
}


void PIDController::Update(mathfu::Quaternionf orientation, float lift, mathfu::Quaternionf q, mathfu::Vector3f w, float elapsed, mathfu::Vector3f& force, mathfu::Vector3f& torque) {
	Quaternionf r = orientation;
	float Fz = lift;

	// calculate error, d(error)/dt, I(error)dt
	e = r*q.Inverse();
	de = -w;
	float eangle;
	Vector3f eaxis;
	e.ToAngleAxis(&eangle, &eaxis);
	ie = eangle > 0.001f ? (elapsed*e)*ie : ie;

	e.Normalize();
	ie.Normalize();

	// calculate desired torque
	Vector3f P, I, D;

	// P
	float pangle;
	Vector3f paxis;
	e.ToAngleAxis(&pangle, &paxis);
	P = pangle > 0.001f ? pangle * paxis : Vector3f{ 0,0,0 };

	// I
	float iangle;
	Vector3f iaxis;
	ie.ToAngleAxis(&iangle, &iaxis);
	I = iangle > 0.001f ? iangle * iaxis : Vector3f{0,0,0};

	// D
	D = de;

	// output
	torque = Kp*P + Ki*I + Kd*D;
	force = q*Vector3f{ 0, 0, 1 };

	file << "r  = [" << r[0] << ", " << r[1] << ", " << r[2] << ", " << r[3] << "]" << endl;
	file << "q  = [" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << "]" << endl;
	file << "w  = [" << w[0] << ", " << w[1] << ", " << w[2] << ", " << w[3] << "]" << endl;
	file << "e  = [" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << "]" << endl;
	file << "de = [" << de[0] << ", " << de[1] << ", " << de[2] << ", " << "]" << endl;
	file << "T  = [" << torque[0] << ", " << torque[1] << ", " << torque[2] << "]" << endl << endl;

	torque = q.Inverse()*torque;

	if (force.z() > 0.5) {
		force.z() = lift / force.z();
	}
	else {
		force.z() = 2 * lift;
	}
}