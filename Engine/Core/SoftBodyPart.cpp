#include "SoftBodyPart.hpp"

using namespace inl::core;

SoftBodyPart::SoftBodyPart(ISoftBodyEntity* e)
:Part(TYPE), entity(e)
{
}

ISoftBodyEntity* SoftBodyPart::GetEntity()
{
	return entity;
}

//void SoftBodyPart::_InnerReflectPos()
//{
//}
//
//void SoftBodyPart::_InnerReflectRot()
//{
//}
//
//void SoftBodyPart::_InnerReflectSkew()
//{
//}