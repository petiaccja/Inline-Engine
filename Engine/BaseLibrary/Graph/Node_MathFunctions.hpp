#pragma once


#include "Node.hpp"

#include <cmath>
#include <cassert>

namespace inl {


template <class ArithmeticT, ArithmeticT(*Function)(ArithmeticT), const char* name>
class MathFunctionNode
	: public InputPortConfig<ArithmeticT>,
	public OutputPortConfig<ArithmeticT>
{
public:
	MathFunctionNode() {
		this->template GetInput<0>().AddObserver(this);
	}

	void Update() override final {
		ArithmeticT a = this->template GetInput<0>().Get();
		this->template GetOutput<0>().Set(Function(a));
	}

	void Notify(InputPortBase* sender) override {
		Update();
	}

	static std::string Info_GetName() {
		return name;
	}
	std::string GetClassName(bool simplify = false, const std::vector<std::regex>& additional = {}) const override {
		auto s = Info_GetName();
		return s.substr(0, s.find_first_of(':'));;
	}

	const std::string& GetInputName(size_t idx) const override {
		assert(idx == 0);
		static const std::string iname = "A";
		return iname;
	}
	const std::string& GetOutputName(size_t idx) const override {
		assert(idx == 0);
		static const std::string oname = "R";
		return oname;
	}
};

struct MathFunctionNames {
public:
	// general
	static const char Abs[];
	// exponential
	static const char Exp[];
	static const char Exp2[];
	static const char Log[];
	static const char Log10[];
	static const char Log2[];
	// power
	static const char Sqrt[];
	static const char Cbrt[];
	// trigonometric
	static const char Sin[];
	static const char Cos[];
	static const char Tan[];
	static const char Asin[];
	static const char Acos[];
	static const char Atan[];
	// hyperbolic
	static const char Sinh[];
	static const char Cosh[];
	static const char Tanh[];
	static const char Asinh[];
	static const char Acosh[];
	static const char Atanh[];
	// statistical
	static const char Erf[];
	static const char Erfc[];
	static const char Gamma[];
	static const char Lgamma[];
	// rounding
	static const char Ceil[];
	static const char Floor[];
	static const char Round[];
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


} // namespace inl
