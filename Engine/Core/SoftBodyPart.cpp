#include "SoftBodyPart.hpp"

using namespace inl::core;

SoftBodyPart::SoftBodyPart(Scene* scene, ISoftBodyEntity* e)
:Part(scene, TYPE), entity(e)
{

}