#pragma once

#include "Node.hpp"

namespace exc {

template <typename OperandT, typename OperatorT, const char* name, const char* op1desc, const char* op2desc, const char* resdesc>
class ComparsionNode
	: public InputPortConfig<OperandT, OperandT>,
	public OutputPortConfig<bool>
{
public:
	ComparsionNode() {
		GetInput<0>().AddObserver(this);
		GetInput<1>().AddObserver(this);
	}

	void Update() override {
		auto in0 = GetInput<0>().Get();
		auto in1 = GetInput<1>().Get();
		GetOutput<0>().Set(OperatorT()(in0, in1));
	}

	void Notify(InputPortBase* sender) override {
		Update();
	}

	static std::string Info_GetName() {
		return name;
	}

	static const std::vector<std::string>& Info_GetInputNames() {
		static std::vector<std::string> names = {
			op1desc,
			op2desc
		};
		return names;
	}
	static const std::vector<std::string>& Info_GetOutputNames() {
		static std::vector<std::string> names = {
			resdesc
		};
		return names;
	}
};

struct EqualStrings {
	static constexpr const char Name[] = "Equal: True if A==B evaluates true.";
	static constexpr const char A[] = "A";
	static constexpr const char B[] = "B";
	static constexpr const char R[] = "R:A?=B";
};

struct NotEqualStrings {
	static constexpr const char Name[] = "NotEqual: True if A!=B evaluates true.";
	static constexpr const char A[] = "A";
	static constexpr const char B[] = "B";
	static constexpr const char R[] = "R:A!=B";
};

struct LessEqualStrings {
	static constexpr const char Name[] = "LessEqual: True if A<=B evaluates true.";
	static constexpr const char A[] = "A";
	static constexpr const char B[] = "B";
	static constexpr const char R[] = "R:A<=B";
};

struct GreaterEqualStrings {
	static constexpr const char Name[] = "GreaterEqual: True if A>=B evaluates true.";
	static constexpr const char A[] = "A";
	static constexpr const char B[] = "B";
	static constexpr const char R[] = "R:A>=B";
};

struct LessStrings {
	static constexpr const char Name[] = "Less: True if A<B evaluates true.";
	static constexpr const char A[] = "A";
	static constexpr const char B[] = "B";
	static constexpr const char R[] = "R:A<B";
};

struct GreaterStrings {
	static constexpr const char Name[] = "Greater: True if A>B evaluates true.";
	static constexpr const char A[] = "A";
	static constexpr const char B[] = "B";
	static constexpr const char R[] = "R:A>B";
};

template <typename T>
using EqualNode = ComparsionNode<
	T,
	std::equal_to<T>,
	EqualStrings::Name,
	EqualStrings::A,
	EqualStrings::B,
	EqualStrings::R>;

template <typename T>
using NotEqualNode = ComparsionNode<
	T,
	std::not_equal_to<T>,
	NotEqualStrings::Name,
	NotEqualStrings::A,
	NotEqualStrings::B,
	NotEqualStrings::R>;

template <typename T>
using LessEqualNode = ComparsionNode<
	T,
	std::less_equal<T>,
	LessEqualStrings::Name,
	LessEqualStrings::A,
	LessEqualStrings::B,
	LessEqualStrings::R>;

template <typename T>
using GreaterEqualNode = ComparsionNode<
	T,
	std::greater_equal<T>,
	GreaterEqualStrings::Name,
	GreaterEqualStrings::A,
	GreaterEqualStrings::B,
	GreaterEqualStrings::R>;

template <typename T>
using LessNode = ComparsionNode<
	T,
	std::less<T>,
	LessStrings::Name,
	LessStrings::A,
	LessStrings::B,
	LessStrings::R>;

template <typename T>
using GreaterNode = ComparsionNode<
	T,
	std::greater<T>,
	GreaterStrings::Name,
	GreaterStrings::A,
	GreaterStrings::B,
	GreaterStrings::R>;


using FloatEqual = EqualNode<float>;
using FloatNotEqual = NotEqualNode<float>;
using FloatLessEqual = LessEqualNode<float>;
using FloatGreaterEqual = GreaterEqualNode<float>;
using FloatLess = LessNode<float>;
using FloatGreater = GreaterNode<float>;

using IntEqual = EqualNode<int>;
using IntNotEqual = NotEqualNode<int>;
using IntLessEqual = LessEqualNode<int>;
using IntGreaterEqual = GreaterEqualNode<int>;
using IntLess = LessNode<int>;
using IntGreater = GreaterNode<int>;


} // namespace exc