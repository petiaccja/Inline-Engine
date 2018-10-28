#include "Node_Arithmetic.hpp"
#include "Node_MathFunctions.hpp"
#include "Node_Comparison.hpp"
#include "Node_Logic.hpp"

#include "NodeFactory.hpp"

namespace inl {

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
template DoubleAdd;
template DoubleSub;
template DoubleMultiply;
template DoubleDivide;
template DoubleModulo;

template LongAdd;
template LongSub;
template LongMultiply;
template LongDivide;
template LongModulo;
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


const char AddStrings::Name[] = "Add:Adds the two inputs";
const char AddStrings::R[] = "R:A+B";
const char AddStrings::A[] = "A";
const char AddStrings::B[] = "B";

const char SubtractStrings::Name[] = "Subtract:Subtracts the second input from the first";
const char SubtractStrings::A[] = "A";
const char SubtractStrings::B[] = "B";
const char SubtractStrings::R[] = "R:A-B";

const char MultiplyStrings::Name[] = "Multiply:Multiplies the two inputs";
const char MultiplyStrings::A[] = "A";
const char MultiplyStrings::B[] = "B";
const char MultiplyStrings::R[] = "R:A*B";

const char DivideStrings::Name[] = "Divide:Divides the first input by the second";
const char DivideStrings::A[] = "A";
const char DivideStrings::B[] = "B";
const char DivideStrings::R[] = "R:A/B";

const char ModuloStrings::Name[] = "Modulo:Divides the first input by the second, and takes the remainder";
const char ModuloStrings::A[] = "A";
const char ModuloStrings::B[] = "B";
const char ModuloStrings::R[] = "R:A%B";

const char PowerStrings::Name[] = "Power:Raises the first input to the power of the second input";
const char PowerStrings::A[] = "Base";
const char PowerStrings::B[] = "Power";
const char PowerStrings::R[] = "R:Base^Power";

const char LogarithmStrings::Name[] = "LogB:Takes the base-<Base> logarithm of Argument";
const char LogarithmStrings::A[] = "Argument";
const char LogarithmStrings::B[] = "Base";
const char LogarithmStrings::R[] = "R:LogBase(Argument)";



const char EqualStrings::Name[] = "Equal: True if A==B evaluates true.";
const char EqualStrings::A[] = "A";
const char EqualStrings::B[] = "B";
const char EqualStrings::R[] = "R:A?=B";

const char NotEqualStrings::Name[] = "NotEqual: True if A!=B evaluates true.";
const char NotEqualStrings::A[] = "A";
const char NotEqualStrings::B[] = "B";
const char NotEqualStrings::R[] = "R:A!=B";

const char LessEqualStrings::Name[] = "LessEqual: True if A<=B evaluates true.";
const char LessEqualStrings::A[] = "A";
const char LessEqualStrings::B[] = "B";
const char LessEqualStrings::R[] = "R:A<=B";

const char GreaterEqualStrings::Name[] = "GreaterEqual: True if A>=B evaluates true.";
const char GreaterEqualStrings::A[] = "A";
const char GreaterEqualStrings::B[] = "B";
const char GreaterEqualStrings::R[] = "R:A>=B";

const char LessStrings::Name[] = "Less: True if A<B evaluates true.";
const char LessStrings::A[] = "A";
const char LessStrings::B[] = "B";
const char LessStrings::R[] = "R:A<B";

const char GreaterStrings::Name[] = "Greater: True if A>B evaluates true.";
const char GreaterStrings::A[] = "A";
const char GreaterStrings::B[] = "B";
const char GreaterStrings::R[] = "R:A>B";



// general
const char MathFunctionNames::Abs[] = "Abs:Absolute value of the input";
// exponential
const char MathFunctionNames::Exp[] = "Exp:Raises e to given power";
const char MathFunctionNames::Exp2[] = "Exp2:Raises 2 to given power";
const char MathFunctionNames::Log[] = "Log:Natural logarithm";
const char MathFunctionNames::Log10[] = "Log10:Base 10 logarithm";
const char MathFunctionNames::Log2[] = "Log2:Base 2 logarithm";
// power
const char MathFunctionNames::Sqrt[] = "Sqrt:Square root of the input";
const char MathFunctionNames::Cbrt[] = "Cbrt:Cubic root of the input";
// trigonometric
const char MathFunctionNames::Sin[] = "Sin:Sine function";
const char MathFunctionNames::Cos[] = "Cos:Cosine function";
const char MathFunctionNames::Tan[] = "Tan:Tangent function";
const char MathFunctionNames::Asin[] = "Asin:Inverse sine function";
const char MathFunctionNames::Acos[] = "Acos:Inverse cosine function";
const char MathFunctionNames::Atan[] = "Atan:Inverse tangent function";
// hyperbolic  
const char MathFunctionNames::Sinh[] = "Sinh:Hyperbolic sine function";
const char MathFunctionNames::Cosh[] = "Cosh:Hyperbolic cosine function";
const char MathFunctionNames::Tanh[] = "Tanh:Hyperbolic tangent function";
const char MathFunctionNames::Asinh[] = "Asinh:Inverse hyperbolic sine function";
const char MathFunctionNames::Acosh[] = "Acosh:Inverse hyperbolic cosine function";
const char MathFunctionNames::Atanh[] = "Atanh:Inverse hyperbolic tangent function";
// statistical
const char MathFunctionNames::Erf[] = "Erf:Error function, the integral of the bell curve";
const char MathFunctionNames::Erfc[] = "Erfc:Complementary error function";
const char MathFunctionNames::Gamma[] = "Gamma:Gamma function";
const char MathFunctionNames::Lgamma[] = "Lgamma:Natural logarithm of the absolute gamma function";
// rounding
const char MathFunctionNames::Ceil[] = "Ceil:The nearest integer not less than input";
const char MathFunctionNames::Floor[] = "Floor:The nearest integer not greater than";
const char MathFunctionNames::Round[] = "Round:The nearest integer";


} // namespace inl


extern "C"
bool g_autoRegisterNodes = [] {
	RegisterIntegerArithmeticNodes(&NodeFactory_Singleton::GetInstance(), "Integer");
	RegisterIntegerComparisonNodes(&NodeFactory_Singleton::GetInstance(), "Integer");
	RegisterFloatArithmeticNodes(&NodeFactory_Singleton::GetInstance(), "Float");
	RegisterFloatComparisonNodes(&NodeFactory_Singleton::GetInstance(), "Float");
	RegisterFloatMathNodes(&NodeFactory_Singleton::GetInstance(), "Float");
	RegisterLogicNodes(&NodeFactory_Singleton::GetInstance(), "Logic");
	return true;
}();