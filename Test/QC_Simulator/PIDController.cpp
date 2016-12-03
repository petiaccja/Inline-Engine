#include "PIDController.hpp"
#include <fstream>
#include <cmath>

using namespace std;
using namespace mathfu;


//static ofstream file("control.txt");


PIDController::PIDController() {
	inertia = mathfu::Matrix3x3f::Identity();

	e = { 1, 0, 0, 0 };
	ie = { 1, 0, 0, 0 };
	de = { 0, 0, 0 };

	Kp = { 40.0f, 40.f, 25.f }; // P
	Ki = { 0.0f, 0.0f, 0.0f }; // I - no integrator
	Kd = { 8.0f, 8.0f, 6.6f }; // D
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
	I = iangle > 0.001f ? iangle * iaxis : Vector3f{ 0,0,0 };

	// D
	D = de;

	// output signal
	Vector3f P_ = q.Inverse()*P,
		I_ = q.Inverse()*I,
		D_ = q.Inverse()*D;
	Vector3f u_ = Kp*P_ + Ki*I_ + Kd*D_;

	// calculate torque via exact linearization
	Vector3f w_ = q.Inverse()*w;
	torque = inertia*u_ + Vector3f::CrossProduct(w_, inertia*w_);

	// calculate force to produce just enough Z-lift in tilted position
	force = q*Vector3f{ 0, 0, 1 };
	if (force.z() > 0.5) {
		force.z() = lift / force.z();
	}
	else {
		force.z() = 2 * lift;
	}
}



//file << "r  = [" << r[0] << ", " << r[1] << ", " << r[2] << ", " << r[3] << "]" << endl;
//file << "q  = [" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << "]" << endl;
//file << "w  = [" << w[0] << ", " << w[1] << ", " << w[2] << "]" << endl;
//file << "e  = [" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << "]" << endl;
//file << "de = [" << de[0] << ", " << de[1] << ", " << de[2] << ", " << "]" << endl;
//file << "T  = [" << torque[0] << ", " << torque[1] << ", " << torque[2] << "]" << endl << endl;