#pragma once

#include "NodeFactory.hpp"

namespace inl {

void RegisterIntegerArithmeticNodes(NodeFactory* factory, const char* group);
void RegisterIntegerComparisonNodes(NodeFactory* factory, const char* group);
void RegisterFloatArithmeticNodes(NodeFactory* factory, const char* group);
void RegisterFloatComparisonNodes(NodeFactory* factory, const char* group);
void RegisterFloatMathNodes(NodeFactory* factory, const char* group);
void RegisterLogicNodes(NodeFactory* factory, const char* group);


} // namespace inl
