#pragma once

#include "Shape.hpp"

#include <optional>
#include <InlineMath.hpp>
#include <BaseLibrary/Transformable.hpp>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>

#undef GetObject // faszom kivan ezzel a kurva winapival


namespace inl::pxeng_bl {


class RigidBody {
public:
	RigidBody();

	void SetPosition(const Vec3& pos);
	Vec3 GetPosition() const;

	void SetRotation(const Quat& rotation);
	Quat GetRotation() const;

	void SetScale(const Vec3& scale);
	Vec3 GetScale() const;

	void SetShape(const Shape* shape);
	const Shape* GetShape() const;

	void SetDynamic(bool dynamic);
	bool IsDynamic() const;

	void UpdateInertia();
	void SetInertia(const Mat33& inertiaTensor);

	btRigidBody* GetObject() const;
private:
	void CalculateInertia();

private:
	Transformable3DN m_transform;
	float m_mass = 1.0f;
	Vec3 m_inertia;
	bool m_autoUpdateInertia = true;

	const Shape* m_shape = nullptr;

	std::unique_ptr<btDefaultMotionState> m_motionState;
	std::unique_ptr<btRigidBody> m_rigidBody;
	std::unique_ptr<btCompoundShape> m_internalShape;
};


}
