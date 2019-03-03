#pragma once

#include "Component.hpp"
#include <BaseLibrary/Transformable.hpp>


namespace inl::game {



class TransformComponent : public Component, public Transformable3DN {
};



} // namespace inl::game