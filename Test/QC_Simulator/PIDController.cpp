#include "PIDController.hpp"
#include <fstream>
#include <cmath>

using namespace std;
using namespace mathfu;
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


void PIDController::Update(mathfu::Quaternionf orientation, float lift, mathfu::Quaternionf q, inl::Vec3 w, float elapsed, inl::Vec3& force, inl::Vec3& torque) {
	Quaternionf r = orientation;
	float Fz = lift;

	// calculate error, d(error)/dt, I(error)dt
	e = r*q.Inverse();
	de = -w;
	float eangle;
	mathfu::Vector<float, 3> eaxis;
	e.ToAngleAxis(&eangle, &eaxis);
	ie = eangle > 0.001f ? (elapsed*e)*ie : ie;

	e.Normalize();
	ie.Normalize();

	// calculate desired torque
	Vec3 P, I, D;

	// P
	float pangle;
	mathfu::Vector<float, 3> paxis;
	e.ToAngleAxis(&pangle, &paxis);
	P = pangle > 0.001f ? pangle * Vec3(paxis.x(), paxis.y(), paxis.z()) : Vec3{ 0,0,0 };

	// I
	float iangle;
	mathfu::Vector<float, 3> iaxis;
	ie.ToAngleAxis(&iangle, &iaxis);
	I = iangle > 0.001f ? iangle * Vec3(iaxis.x(), iaxis.y(), iaxis.z()) : Vec3{ 0,0,0 };

	// D
	D = de;

	// output signal
	mathfu::Vector<float, 3>
		P_ = q.Inverse()*mathfu::Vector<float, 3>(P.x, P.y, P.z),
		I_ = q.Inverse()* mathfu::Vector<float, 3>(I.x, I.y, I.z),
		D_ = q.Inverse()* mathfu::Vector<float, 3>(D.x, D.y, D.z);
	Vec3 u_ = Kp*Vec3(P_.x(), P_.y(), P_.z()) + Ki*Vec3(I_.x(), I_.y(), I_.z()) + Kd*Vec3(D_.x(), D_.y(), D_.z());

	// calculate torque via exact linearization
	mathfu::Vector<float, 3> w_tmp = q.Inverse()*mathfu::Vector<float, 3>(w.x, w.y, w.z);
	Vec3 w_ = { w_tmp.x(), w_tmp.y(), w_tmp.z() };
	torque = inertia*u_ + Cross(w_, inertia*w_);

	// calculate force to produce just enough Z-lift in tilted position
	auto force_tmp = q*mathfu::Vector<float, 3>{ 0, 0, 1 };
	force = { force_tmp.x(), force_tmp.y(), force_tmp.z() };
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