#pragma once


#include <mathfu/mathfu_exc.hpp>


class RigidBody {
public:
	void Update(float timestep, mathfu::Vector3f F_, mathfu::Vector3f T_);

	void SetPosition(mathfu::Vector3f p) { this->p = p; }
	void SetRotation(mathfu::Quaternionf q) { this->q = q; }
	void Reset();

	mathfu::Vector3f Position() const { return p; }
	mathfu::Quaternionf Rotation() const { return q; }
private:
	mathfu::Vector3f p;
	mathfu::Vector3f v;

	mathfu::Quaternionf q;
	mathfu::Quaternionf dq;
	mathfu::Vector3f w_;
};
