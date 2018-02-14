#include "PIDController.hpp"
#include <fstream>
#include <cmath>
#include <iostream>

using namespace std;
using namespace inl;

//static ofstream file("control.txt");


PIDController::PIDController() {
	inertia = inl::Mat33::Identity();

	e = { 1, 0, 0, 0 };
	ie = { 1, 0, 0, 0 };
	de = { 0, 0, 0 };

	Kp = { 40.0f, 40.f, 25.f }; // P
	Ki = { 0.0f, 0.0f, 0.0f }; // I - no integrator
	Kd = { 8.0f, 8.0f, 6.6f }; // D
}


void PIDController::Update(inl::Quat orientation, float lift, inl::Quat q, inl::Vec3 w, float elapsed, inl::Vec3& force, inl::Vec3& torque) {
	Quat r = orientation;
	float Fz = lift;

	// calculate error, d(error)/dt, I(error)dt
	e = r*q.Inverse();
	de = -w;
	float eangle;
	Vec3 eaxis;
	eaxis = e.Axis();
	eangle = e.Angle();
	ie = (elapsed*e)*ie;

	e.Normalize();
	ie.Normalize();

	// calculate desired torque
	Vec3 P, I, D;

	// P
	float pangle;
	Vec3 paxis;
	paxis = e.Axis();
	pangle = e.Angle();
	P = pangle * paxis;

	// I
	float iangle;
	Vec3 iaxis;
	iaxis = ie.Axis();
	iangle = ie.Angle();
	I = iangle * iaxis;

	// D
	D = de;

	// output signal
	Vec3
		P_ = q.Inverse()*P,
		I_ = q.Inverse()*I,
		D_ = q.Inverse()*D;
	Vec3 u_ = Kp*P_ + Ki*I_ + Kd*D_;

	// calculate torque via exact linearization
	//mathfu::Vector<float, 3> w_tmp = q.Inverse()*mathfu::Vector<float, 3>(w.x, w.y, w.z);
	//Vec3 w_ = { w_tmp.x(), w_tmp.y(), w_tmp.z() };
	Vec3 w_ = q.Inverse()*w;
	torque = inertia*u_ + Cross(w_, inertia*w_);

	// calculate force to produce just enough Z-lift in tilted position
	force = q*Vec3{ 0, 0, 1 };
	if (force.z > 0.5) {
		force.z = lift / force.z;
	}
	else {
		force.z = 2 * lift;
	}
}



//file << "r  = [" << r[0] << ", " << r[1] << ", " << r[2] << ", " << r[3] << "]" << endl;
//file << "q  = [" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << "]" << endl;
//file << "w  = [" << w[0] << ", " << w[1] << ", " << w[2] << "]" << endl;
//file << "e  = [" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << "]" << endl;
//file << "de = [" << de[0] << ", " << de[1] << ", " << de[2] << ", " << "]" << endl;
//file << "T  = [" << torque[0] << ", " << torque[1] << ", " << torque[2] << "]" << endl << endl;