#include "SoftBodyPart.hpp"

using namespace inl::core;

SoftBodyPart::SoftBodyPart(ISoftBodyEntity* e)
:Part(TYPE), entity(e)
{
}