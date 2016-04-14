#pragma once


#include "Node.hpp"

#include <cmath>

namespace exc {


template <class ArithmeticT, ArithmeticT(*Function)(ArithmeticT), const char* name>
class MathFunctionNode
	: public InputPortConfig<ArithmeticT>,
	public OutputPortConfig<ArithmeticT>
{
public:
	MathFunctionNode() {
		GetInput<0>().AddObserver(this);
	}

	void Update() override final {
		ArithmeticT a = GetInput<0>().Get();
		GetOutput<0>().Set(Function(a));
	}

	void Notify(InputPortBase* sender) override {
		Update();
	}

	static std::string Info_GetName() {
		return name;
	}

	static const std::vector<std::string>& Info_GetInputNames() {
		static std::vector<std::string> names = {
			"A"
		};
		return names;
	}
	static const std::vector<std::string>& Info_GetOutputNames() {
		static std::vector<std::string> names = {
			"R"
		};
		return names;
	}
};

struct MathFunctionNames {
public:
	// general
	static constexpr const char Abs[] = "Abs:Absolute value of the input";
	// exponential
	static constexpr const char Exp[] = "Exp:Raises e to given power";
	static constexpr const char Exp2[] = "Exp2:Raises 2 to given power";
	static constexpr const char Log[] = "Log:Natural logarithm";
	static constexpr const char Log10[] = "Log10:Base 10 logarithm";
	static constexpr const char Log2[] = "Log2:Base 2 logarithm";
	// power
	static constexpr const char Sqrt[] = "Sqrt:Square root of the input";
	static constexpr const char Cbrt[] = "Cbrt:Cubic root of the input";
	// trigonometric
	static constexpr const char Sin[] = "Sin:Sine function";
	static constexpr const char Cos[] = "Cos:Cosine function";
	static constexpr const char Tan[] = "Tan:Tangent function";
	static constexpr const char Asin[] = "Asin:Inverse sine function";
	static constexpr const char Acos[] = "Acos:Inverse cosine function";
	static constexpr const char Atan[] = "Atan:Inverse tangent function";
	// hyperbolic
	static constexpr const char Sinh[] = "Sinh:Hyperbolic sine function";
	static constexpr const char Cosh[] = "Cosh:Hyperbolic cosine function";
	static constexpr const char Tanh[] = "Tanh:Hyperbolic tangent function";
	static constexpr const char Asinh[] = "Asinh:Inverse hyperbolic sine function";
	static constexpr const char Acosh[] = "Acosh:Inverse hyperbolic cosine function";
	static constexpr const char Atanh[] = "Atanh:Inverse hyperbolic tangent function";
	// statistical
	static constexpr const char Erf[] = "Erf:Error function, the integral of the bell curve";
	static constexpr const char Erfc[] = "Erfc:Complementary error function";
	static constexpr const char Gamma[] = "Gamma:Gamma function";
	static constexpr const char Lgamma[] = "Lgamma:Natural logarithm of the absolute gamma function";
	// rounding
	static constexpr const char Ceil[] = "Ceil:The nearest integer not less than input";
	static constexpr const char Floor[] = "Floor:The nearest integer not greater than";
	static constexpr const char Round[] = "Round:The nearest integer";
};

// general
using FloatAbs = MathFunctionNode<float, std::abs, MathFunctionNames::Abs>;
// exponential
using FloatExp = MathFunctionNode<float, std::exp, MathFunctionNames::Exp>;
using FloatExp2 = MathFunctionNode<float, std::exp2, MathFunctionNames::Exp2>;
using FloatLog = MathFunctionNode<float, std::log, MathFunctionNames::Log>;
using FloatLog10 = MathFunctionNode<float, std::log10, MathFunctionNames::Log10>;
using FloatLog2 = MathFunctionNode<float, std::log2, MathFunctionNames::Log2>;
// power
using FloatSqrt = MathFunctionNode<float, std::sqrt, MathFunctionNames::Sqrt>;
using FloatCbrt = MathFunctionNode<float, std::cbrt, MathFunctionNames::Cbrt>;
// trigonometric
using FloatSin = MathFunctionNode<float, std::sin, MathFunctionNames::Sin>;
using FloatCos = MathFunctionNode<float, std::cos, MathFunctionNames::Cos>;
using FloatTan = MathFunctionNode<float, std::tan, MathFunctionNames::Tan>;
using FloatAsin = MathFunctionNode<float, std::asin, MathFunctionNames::Asin>;
using FloatAcos = MathFunctionNode<float, std::acos, MathFunctionNames::Acos>;
using FloatAtan = MathFunctionNode<float, std::atan, MathFunctionNames::Atan>;
// hyperbolic
using FloatSinh = MathFunctionNode<float, std::sinh, MathFunctionNames::Sinh>;
using FloatCosh = MathFunctionNode<float, std::cosh, MathFunctionNames::Cosh>;
using FloatTanh = MathFunctionNode<float, std::tanh, MathFunctionNames::Tanh>;
using FloatAsinh = MathFunctionNode<float, std::asinh, MathFunctionNames::Asinh>;
using FloatAcosh = MathFunctionNode<float, std::acosh, MathFunctionNames::Acosh>;
using FloatAtanh = MathFunctionNode<float, std::atanh, MathFunctionNames::Atanh>;
// statistical
using FloatErf = MathFunctionNode<float, std::erf, MathFunctionNames::Erf>;
using FloatErfc = MathFunctionNode<float, std::erfc, MathFunctionNames::Erfc>;
using FloatGamma = MathFunctionNode<float, std::tgamma, MathFunctionNames::Gamma>;
using FloatLgamma = MathFunctionNode<float, std::lgamma, MathFunctionNames::Lgamma>;
// rounding
using FloatCeil = MathFunctionNode<float, std::ceil, MathFunctionNames::Ceil>;
using FloatFloor = MathFunctionNode<float, std::floor, MathFunctionNames::Floor>;
using FloatRound = MathFunctionNode<float, std::round, MathFunctionNames::Round>;


} // namespace exc