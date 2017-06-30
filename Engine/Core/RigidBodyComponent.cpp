#include "RigidBodyComponent.hpp"

//void RigidBodyComponent::UpdateAfterPhysicsSimulate()
//{
//	WorldComponent::SetPos(entity->GetPos());
//	WorldComponent::SetRot(entity->GetRot());
//}

//void RigidBodyComponent::_InnerReflectPos()
//{
//	entity->SetPos(transform.GetPos());
//}
//
//void RigidBodyComponent::_InnerReflectRot()
//{
//	entity->SetRot(transform.GetRot());
//}
//
//void RigidBodyComponent::_InnerReflectSkew()
//{
//	entity->SetScale(transform.GetScale());
//	//entity->SetSkew(transform.GetSkew());
//}

RigidBodyComponent::RigidBodyComponent(physics::IRigidBodyEntity* a)
:WorldComponent(TYPE), entity(a)
{

}

void RigidBodyComponent::AddForce(const Vec3& force, const Vec3& relPos /*= { 0, 0, 0 }*/)
{
	entity->AddForce(force, relPos);
}

void RigidBodyComponent::SetUserPointer(void* p)
{
	entity->SetUserPointer(p);
}

void RigidBodyComponent::SetGravityScale(float s)
{
	entity->SetGravityScale(s);
}

void RigidBodyComponent::SetTrigger(bool bTrigger)
{
	entity->SetTrigger(bTrigger);
}

void RigidBodyComponent::SetCollisionGroup(uint64_t ID)
{
	entity->SetCollisionGroup(ID);
}

void RigidBodyComponent::SetAngularFactor(float factor)
{
	entity->SetAngularFactor(factor);
}

void RigidBodyComponent::SetKinematic(bool bKinematic)
{
	entity->SetKinematic(bKinematic);
}

void RigidBodyComponent::SetVelocity(const Vec3& v)
{
	entity->SetVelocity(v);
}

uint64_t RigidBodyComponent::GetCollisionGroup() const
{
	return entity->GetCollisionGroup();
}

Vec3 RigidBodyComponent::GetVelocity() const
{
	return entity->GetVelocity();
}

void* RigidBodyComponent::GetUserPointer()
{
	return entity->GetUserPointer();
}

bool RigidBodyComponent::IsTrigger() const
{
	return entity->IsTrigger();
}

bool RigidBodyComponent::IsStatic() const
{
	return entity->IsStatic();
}

bool RigidBodyComponent::IsDynamic() const
{
	return entity->IsDynamic();
}

bool RigidBodyComponent::IsKinematic() const
{
	return entity->IsKinematic();
}

physics::IRigidBodyEntity* RigidBodyComponent::GetEntity()
{
	return entity;
}

std::vector<physics::ContactPoint> RigidBodyComponent::GetContactPoints() const
{
	return entity->GetContactPoints();
}