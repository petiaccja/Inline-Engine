#include "SoftBodyComponent.hpp"

using namespace inl::core;

SoftBodyComponent::SoftBodyComponent(physics::ISoftBodyEntity* e)
:WorldComponent(TYPE), entity(e)
{
}

physics::ISoftBodyEntity* SoftBodyComponent::GetEntity()
{
	return entity;
}

//void SoftBodyComponent::_InnerReflectPos()
//{
//}
//
//void SoftBodyComponent::_InnerReflectRot()
//{
//}
//
//void SoftBodyComponent::_InnerReflectSkew()
//{
//}