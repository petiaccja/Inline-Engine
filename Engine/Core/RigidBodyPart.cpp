#include "RigidBodyPart.hpp"

using namespace inl::core;

//void RigidBodyPart::UpdateAfterPhysicsSimulate()
//{
//	Part::SetPos(entity->GetPos());
//	Part::SetRot(entity->GetRot());
//}

//void RigidBodyPart::_InnerReflectPos()
//{
//	entity->SetPos(transform.GetPos());
//}
//
//void RigidBodyPart::_InnerReflectRot()
//{
//	entity->SetRot(transform.GetRot());
//}
//
//void RigidBodyPart::_InnerReflectSkew()
//{
//	entity->SetScale(transform.GetScale());
//	//entity->SetSkew(transform.GetSkew());
//}

RigidBodyPart::RigidBodyPart(physics::IRigidBodyEntity* a)
:Part(TYPE), entity(a)
{

}

void RigidBodyPart::ApplyForce(const Vec3& force, const Vec3& relPos /*= { 0, 0, 0 }*/)
{
	entity->ApplyForce(force, relPos);
}

void RigidBodyPart::SetUserPointer(void* p)
{
	entity->SetUserPointer(p);
}

void RigidBodyPart::SetGravityScale(float s)
{
	entity->SetGravityScale(s);
}

void RigidBodyPart::SetTrigger(bool bTrigger)
{
	entity->SetTrigger(bTrigger);
}

void RigidBodyPart::SetCollisionGroup(uint64_t ID)
{
	entity->SetCollisionGroup(ID);
}

void RigidBodyPart::SetAngularFactor(float factor)
{
	entity->SetAngularFactor(factor);
}

void RigidBodyPart::SetKinematic(bool bKinematic)
{
	entity->SetKinematic(bKinematic);
}

void RigidBodyPart::SetVelocity(const Vec3& v)
{
	entity->SetVelocity(v);
}

uint64_t RigidBodyPart::GetCollisionGroup() const
{
	return entity->GetCollisionGroup();
}

Vec3 RigidBodyPart::GetVelocity() const
{
	return entity->GetVelocity();
}

void* RigidBodyPart::GetUserPointer()
{
	return entity->GetUserPointer();
}

bool RigidBodyPart::IsTrigger() const
{
	return entity->IsTrigger();
}

bool RigidBodyPart::IsStatic() const
{
	return entity->IsStatic();
}

bool RigidBodyPart::IsDynamic() const
{
	return entity->IsDynamic();
}

bool RigidBodyPart::IsKinematic() const
{
	return entity->IsKinematic();
}

std::vector<Contact> RigidBodyPart::GetContacts() const
{
	return entity->GetContacts();
}