#include "Node_Arithmetic.hpp"
#include "Node_MathFunctions.hpp"
#include "Node_Comparison.hpp"
#include "Node_Logic.hpp"

#include "NodeFactory.hpp"

namespace exc {

// Explicit instantiations, let them compile.

// Basic arithmetic
template FloatAdd;
template FloatSubtract;
template FloatMultiply;
template FloatDivide;
template FloatModulo;
template FloatPower;
template FloatLogarithm;

template IntAdd;
template IntSubtract;
template IntMultiply;
template IntDivide;
template IntModulo;

#ifdef ENABLE_HIGH_PRECISION_NODES
template FloatAdd;
template FloatSub;
template FloatMultiply;
template FloatDivide;
template FloatModulo;

template FloatAdd;
template FloatSub;
template FloatMultiply;
template FloatDivide;
template FloatModulo;
#endif

// Comparison
template FloatEqual;
template FloatNotEqual;
template FloatLessEqual;
template FloatGreaterEqual;
template FloatLess;
template FloatGreater;

template IntEqual;
template IntNotEqual;
template IntLessEqual;
template IntGreaterEqual;
template IntLess;
template IntGreater;

// Mathematical functions
template FloatAbs;

template FloatExp;
template FloatExp2;
template FloatLog;
template FloatLog10;
template FloatLog2;

template FloatSqrt;
template FloatCbrt;

template FloatSin;
template FloatCos;
template FloatTan;
template FloatAsin;
template FloatAcos;
template FloatAtan;

template FloatSinh;
template FloatCosh;
template FloatTanh;
template FloatAsinh;
template FloatAcosh;
template FloatAtanh;

template FloatErf;
template FloatErfc;
template FloatGamma;
template FloatLgamma;

template FloatCeil;
template FloatFloor;
template FloatRound;


// Manual register functions

void RegisterIntegerArithmeticNodes(NodeFactory* factory, const char* group) {
	factory->RegisterNodeClass<IntAdd>(group);
	factory->RegisterNodeClass<IntSubtract>(group);
	factory->RegisterNodeClass<IntMultiply>(group);
	factory->RegisterNodeClass<IntDivide>(group);
	factory->RegisterNodeClass<IntModulo>(group);
}


void RegisterIntegerComparisonNodes(NodeFactory* factory, const char* group) {
	factory->RegisterNodeClass<IntEqual>(group);
	factory->RegisterNodeClass<IntNotEqual>(group);
	factory->RegisterNodeClass<IntLessEqual>(group);
	factory->RegisterNodeClass<IntGreaterEqual>(group);
	factory->RegisterNodeClass<IntLess>(group);
	factory->RegisterNodeClass<IntGreater>(group);
}


void RegisterFloatArithmeticNodes(NodeFactory* factory, const char* group) {
	factory->RegisterNodeClass<FloatAdd>(group);
	factory->RegisterNodeClass<FloatSubtract>(group);
	factory->RegisterNodeClass<FloatMultiply>(group);
	factory->RegisterNodeClass<FloatDivide>(group);
	factory->RegisterNodeClass<FloatModulo>(group);
	factory->RegisterNodeClass<FloatPower>(group);
	factory->RegisterNodeClass<FloatLogarithm>(group);
}

void RegisterFloatComparisonNodes(NodeFactory* factory, const char* group) {
	factory->RegisterNodeClass<FloatEqual>(group);
	factory->RegisterNodeClass<FloatNotEqual>(group);
	factory->RegisterNodeClass<FloatLessEqual>(group);
	factory->RegisterNodeClass<FloatGreaterEqual>(group);
	factory->RegisterNodeClass<FloatLess>(group);
	factory->RegisterNodeClass<FloatGreater>(group);
}


void RegisterFloatMathNodes(NodeFactory* factory, const char* group) {
	factory->RegisterNodeClass<FloatAbs>(group);

	factory->RegisterNodeClass<FloatExp>(group);
	factory->RegisterNodeClass<FloatExp2>(group);
	factory->RegisterNodeClass<FloatLog>(group);
	factory->RegisterNodeClass<FloatLog10>(group);
	factory->RegisterNodeClass<FloatLog2>(group);

	factory->RegisterNodeClass<FloatSqrt>(group);
	factory->RegisterNodeClass<FloatCbrt>(group);

	factory->RegisterNodeClass<FloatSin>(group);
	factory->RegisterNodeClass<FloatCos>(group);
	factory->RegisterNodeClass<FloatTan>(group);
	factory->RegisterNodeClass<FloatAsin>(group);
	factory->RegisterNodeClass<FloatAcos>(group);
	factory->RegisterNodeClass<FloatAtan>(group);

	factory->RegisterNodeClass<FloatSinh>(group);
	factory->RegisterNodeClass<FloatCosh>(group);
	factory->RegisterNodeClass<FloatTanh>(group);
	factory->RegisterNodeClass<FloatAsinh>(group);
	factory->RegisterNodeClass<FloatAcosh>(group);
	factory->RegisterNodeClass<FloatAtanh>(group);

	factory->RegisterNodeClass<FloatErf>(group);
	factory->RegisterNodeClass<FloatErfc>(group);
	factory->RegisterNodeClass<FloatGamma>(group);
	factory->RegisterNodeClass<FloatLgamma>(group);

	factory->RegisterNodeClass<FloatCeil>(group);
	factory->RegisterNodeClass<FloatFloor>(group);
	factory->RegisterNodeClass<FloatRound>(group);
}


void RegisterLogicNodes(NodeFactory* factory, const char* group) {
	factory->RegisterNodeClass<LogicAny>(group);
	factory->RegisterNodeClass<LogicAll>(group);
}


} // namespace exc