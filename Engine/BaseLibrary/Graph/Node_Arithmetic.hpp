#pragma once


#include "Node.hpp"

namespace inl {


template <class ArithmeticT, class Operator, const char* name, const char* op1desc, const char* op2desc, const char* resdesc>
class BinaryArithmeticNode
	: public InputPortConfig<ArithmeticT, ArithmeticT>,
	public OutputPortConfig < ArithmeticT >
{
public:
	BinaryArithmeticNode() {
		this->template GetInput<0>().AddObserver(this);
		this->template GetInput<1>().AddObserver(this);
	}

	void Update() override {
		ArithmeticT a = this->template GetInput<0>().Get();
		ArithmeticT b = this->template GetInput<1>().Get();
		this->template GetOutput<0>().Set(Operator()(a, b));
	}

	void Notify(InputPortBase* sender) override {
		Update();
	}

	static std::string Info_GetName() {
		return name;
	}
	std::string GetClassName(bool simplify = false, const std::vector<std::regex>& additional = {}) const override {
		auto s = Info_GetName();
		return s.substr(0, s.find_first_of(':'));
	}

	const std::string& GetInputName(size_t idx) const override {
		static std::vector<std::string> names = {
			op1desc,
			op2desc
		};
		return names[idx];
	}
	const std::string& GetOutputName(size_t idx) const override {
		static std::vector<std::string> names = {
			resdesc
		};
		return names[idx];
	}
};

// Operators
struct AddOperator {
	template <class T>
	auto operator()(T t, T u) {
		return t + u;
	}
};

struct SubtractOperator {
	template <class T>
	auto operator()(T t, T u) {
		return t - u;
	}
};

struct MultiplyOperator {
	template <class T>
	auto operator()(T t, T u) {
		return t * u;
	}
};

struct DivideOperator {
	template <class T>
	auto operator()(T t, T u) {
		return t / u;
	}
};

struct ModuloOperator {
public:
	template <class T>
	auto operator()(T t, T u) {
		if constexpr (std::is_floating_point<T>::value) {
			return std::remainder(t, u);
		}
		else {
			return t % u;
		}
	}
};

struct PowerOperator {
	template <class T>
	T operator()(T t, T u) {
		return std::pow(t, u);
	}
};

struct LogarithmOperator {
	template <class T>
	T operator()(T t, T u) {
		return std::log(t) / std::log(u);
	}
};



// Strings
struct AddStrings {
	static const char Name[];
	static const char R[];
	static const char A[];
	static const char B[];
};

struct SubtractStrings {
	static const char Name[];
	static const char A[];
	static const char B[];
	static const char R[];
};

struct MultiplyStrings {
	static const char Name[];
	static const char A[];
	static const char B[];
	static const char R[];
};

struct DivideStrings {
	static const char Name[];
	static const char A[];
	static const char B[];
	static const char R[];
};

struct ModuloStrings {
	static const char Name[];
	static const char A[];
	static const char B[];
	static const char R[];
};

struct PowerStrings {
	static const char Name[];
	static const char A[];
	static const char B[];
	static const char R[];
};

struct LogarithmStrings {
	static const char Name[];
	static const char A[];
	static const char B[];
	static const char R[];
};

// Node templates
template <class T>
using AddNode = BinaryArithmeticNode<
	T,
	AddOperator,
	AddStrings::Name,
	AddStrings::A,
	AddStrings::B,
	AddStrings::R>;

template <class T>
using SubtractNode = BinaryArithmeticNode<
	T,
	SubtractOperator,
	SubtractStrings::Name,
	SubtractStrings::A,
	SubtractStrings::B,
	SubtractStrings::R>;

template <class T>
using MultiplyNode = BinaryArithmeticNode<
	T,
	MultiplyOperator,
	MultiplyStrings::Name,
	MultiplyStrings::A,
	MultiplyStrings::B,
	MultiplyStrings::R>;

template <class T>
using DivideNode = BinaryArithmeticNode<
	T,
	DivideOperator,
	DivideStrings::Name,
	DivideStrings::A,
	DivideStrings::B,
	DivideStrings::R>;

template <class T>
using ModuloNode = BinaryArithmeticNode<
	T,
	ModuloOperator,
	ModuloStrings::Name,
	ModuloStrings::A,
	ModuloStrings::B,
	ModuloStrings::R>;

template <class T>
using PowerNode = BinaryArithmeticNode<
	T,
	PowerOperator,
	PowerStrings::Name,
	PowerStrings::A,
	PowerStrings::B,
	PowerStrings::R>;

template <class T>
using LogarithmNode = BinaryArithmeticNode<
	T,
	LogarithmOperator,
	LogarithmStrings::Name,
	LogarithmStrings::A,
	LogarithmStrings::B,
	LogarithmStrings::R>;

// Actual nodes
using FloatAdd = AddNode<float>;
using FloatSubtract = SubtractNode<float>;
using FloatMultiply = MultiplyNode<float>;
using FloatDivide = DivideNode<float>;
using FloatModulo = ModuloNode<float>;
using FloatPower = PowerNode<float>;
using FloatLogarithm = LogarithmNode<float>;

using IntAdd = AddNode<int>;
using IntSubtract = SubtractNode<int>;
using IntMultiply = MultiplyNode<int>;
using IntDivide = DivideNode<int>;
using IntModulo = ModuloNode<int>;

#ifdef ENABLE_HIGH_PRECISION_NODES
using DoubleAdd = AddNode<double>;
using DoubleSub = SubtractNode<double>;
using DoubleMultiply = MultiplyNode<double>;
using DoubleDivide = DivideNode<double>;
using DoubleModulo = ModuloNode<double>;

using LongAdd = AddNode<long long>;
using LongSub = SubtractNode<long long>;
using LongMultiply = MultiplyNode<long long>;
using LongDivide = DivideNode<long long>;
using LongModulo = ModuloNode<long long>;
#endif



} // namespace inl
