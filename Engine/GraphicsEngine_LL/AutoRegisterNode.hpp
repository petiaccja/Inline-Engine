#pragma once

#include "GraphicsNodeFactory.hpp"

#include <iostream>
#include <typeinfo>


#define INL_REGISTER_GRAPHICS_NODE(ClassName)                                          \
	const int __declspec(dllexport) s_registrar_##ClassName = [] {                     \
		GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<ClassName>(""); \
		return true;                                                                   \
	}();


#define INL_REGISTER_FORCE(ClassName) INL_NODE_FORCE_INCLUDE(s_registrar_##ClassName)