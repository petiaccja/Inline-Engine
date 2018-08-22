#include "RigidBody.hpp"
#include "BulletTypes.hpp"
#include <BaseLibrary/Exception/Exception.hpp>


#undef GetObject // retarded winapi


namespace inl::pxeng_bl {



RigidBody::RigidBody()
{
	m_motionState = std::make_unique<btDefaultMotionState>();
	m_internalShape = std::make_unique<btCompoundShape>();
	m_rigidBody = std::make_unique<btRigidBody>(1.0f, m_motionState.get(), m_internalShape.get());
}


void RigidBody::SetPosition(const Vec3& pos) {
	m_transform.SetPosition(pos);

	btTransform transform = m_rigidBody->getWorldTransform();
	transform.setOrigin(Conv(pos));
	m_rigidBody->setWorldTransform(transform);
}

void RigidBody::SetRotation(const Quat& rotation) {
	m_transform.SetRotation(rotation);

	btTransform transform = m_rigidBody->getWorldTransform();
	transform.setRotation(Conv(m_transform.GetPostRotation()));
	m_rigidBody->setWorldTransform(transform);
}

void RigidBody::SetScale(const Vec3& scale) {
	m_transform.SetScale(scale);

	m_internalShape->setLocalScaling(Conv(scale));
}


Vec3 RigidBody::GetPosition() const {
	return m_transform.GetPosition();
}

Quat RigidBody::GetRotation() const {
	return m_transform.GetRotation();
}

Vec3 RigidBody::GetScale() const {
	return m_transform.GetScale();
}


void RigidBody::SetShape(const Shape* shape) {
	m_shape = shape;

	// Remove all child shapes from internal compound shape.
	int numChildShapes = m_internalShape->getNumChildShapes();
	assert(numChildShapes <= 1); // Compound shape is only to apply scale/shear, not to combine.
	if (numChildShapes == 1) {
		m_internalShape->removeChildShapeByIndex(0);
	}


	// Figure out pre-transform.
	Quat r1 = m_transform.GetShearRotation();
	Vec3 s = m_transform.GetScale();
	Quat r2 = m_transform.GetRotation();
	Vec3 p = m_transform.GetPosition();
	
	// Add new shape to compound shape with pre-rotation.
	btTransform transform;
	transform.setRotation(Conv(r1));
	m_internalShape->addChildShape(transform, shape->GetInternalShape());

	// Set shear scale to compound transform.
	m_internalShape->setLocalScaling(Conv(s));

	// Set post transform to entity.
	btTransform bodyTransform;
	bodyTransform.setRotation(Conv(r2));
	bodyTransform.setOrigin(Conv(p));
	m_rigidBody->setWorldTransform(bodyTransform);

	// Update inertia.
	if (m_autoUpdateInertia) {
		CalculateInertia();
	}
}
const Shape* RigidBody::GetShape() const {
	return m_shape;
}


void RigidBody::SetDynamic(bool dynamic) {
	if (dynamic) {
		m_rigidBody->setMassProps(m_mass, Conv(m_inertia));
	}
	else {
		m_rigidBody->setMassProps(0.0f, { 0.0f, 0.0f, 0.0f });
	}
}
bool RigidBody::IsDynamic() const {
	float mass = 1.0f / m_rigidBody->getInvMass();
	return mass > 0.0f;
}


void RigidBody::SetInertia(const Mat33& inertiaTensor) {
	throw NotImplementedException();
}

btRigidBody* RigidBody::GetObject() const {
	return m_rigidBody.get();
}


void RigidBody::CalculateInertia() {
	btVector3 inertia;
	m_internalShape->calculateLocalInertia(m_mass, inertia);
	m_inertia = Conv(inertia);
	if (IsDynamic()) {
		m_rigidBody->setMassProps(m_mass, inertia);
	}
}


} // inl::pxeng_bl
