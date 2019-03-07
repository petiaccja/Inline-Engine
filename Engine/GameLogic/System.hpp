#pragma once

#include "Component.hpp"


namespace inl::gxeng {


template <class... Components>
class System;


template <class Component1, class Component2, class... Components>
class System<Component1, Component2, Components...> {
	
};


} // namespace inl::gxeng