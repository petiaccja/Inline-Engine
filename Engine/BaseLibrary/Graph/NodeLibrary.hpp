#pragma once

#include "NodeFactory.hpp"

namespace inl {

void RegisterIntegerArithmeticNodes(NodeFactory* factory, const char* group);
void RegisterIntegerComparisonNodes(NodeFactory* factory, const char* group);
void RegisterFloatArithmeticNodes(NodeFactory* factory, const char* group);
void RegisterFloatComparisonNodes(NodeFactory* factory, const char* group);
void RegisterFloatMathNodes(NodeFactory* factory, const char* group);
void RegisterLogicNodes(NodeFactory* factory, const char* group);


#if defined(_WIN32)
# if defined(_WIN64)
#  define INL_NODE_FORCE_INCLUDE(x) __pragma(comment (linker, "/export:" #x))
# else
#  define INL_NODE_FORCE_INCLUDE(x) __pragma(comment (linker, "/export:_" #x))
# endif
#else
# define INL_NODE_FORCE_INCLUDE(x) extern "C" void x(void); void (*__ ## x ## _fp)(void)=&x;
#endif


} // namespace inl

extern "C"
extern bool g_autoRegisterNodes;
#define INL_NODE_FORCE_REGISTER INL_NODE_FORCE_INCLUDE(g_autoRegisterNodes)
