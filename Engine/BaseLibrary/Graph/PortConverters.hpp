#pragma once

#include "Port.hpp"
#include <type_traits>
#include <cstdlib>
#include <string>
#include <InlineMath.hpp>


namespace inl {


template <class AritT>
class PortConverterArithmetic : public PortConverterCollection<AritT> {
protected:
	static_assert(std::is_arithmetic<AritT>::value, "Type must be arithmetic.");

	PortConverterArithmetic() : PortConverterCollection<AritT>(
		&FromArithmetic<char>,
		&FromArithmetic<unsigned char>,
		&FromArithmetic<signed char>,
		&FromArithmetic<short>,
		&FromArithmetic<unsigned short>,
		&FromArithmetic<int>,
		&FromArithmetic<unsigned int>,
		&FromArithmetic<long>,
		&FromArithmetic<unsigned long>,
		&FromArithmetic<long long>,
		&FromArithmetic<unsigned long long>,
		&FromArithmetic<float>,
		&FromArithmetic<double>,
		&FromArithmetic<long double>,
		&FromString)
	{}

	template <class SourceT>
	static AritT FromArithmetic(SourceT src) {
		return AritT(src);
	}


	static AritT FromString(const std::string& str) {
		if (str.empty()) {
			throw InvalidCastException("Cannot convert empty string to artihmetic.");
		}
		char* pend;
		AritT value;
		if constexpr (std::is_integral_v<AritT> && std::is_signed_v<AritT>) {
			value = (AritT)strtoll(str.c_str(), &pend, 10);
		}
		else if (std::is_integral_v<AritT> && !std::is_signed_v<AritT>) {
			value = (AritT)strtoull(str.c_str(), &pend, 10);
		}
		else if (std::is_floating_point_v<AritT>) {
			value = (AritT)strtold(str.c_str(), &pend);
		}
		else {
			//static_assert(impl::dependent_false_v<AritT>, "Do not know how to convert non-arithmetic type.");
		}
		if (*pend != '\0') {
			throw InvalidCastException("Invalid number format.");
		}
		return value;
	}
};


template <>
class PortConverter<char> : public PortConverterArithmetic<char>
{};
template <>
class PortConverter<unsigned char> : public PortConverterArithmetic<unsigned char>
{};
template <>
class PortConverter<signed char> : public PortConverterArithmetic<signed char>
{};
template <>
class PortConverter<short> : public PortConverterArithmetic<short>
{};
template <>
class PortConverter<unsigned short> : public PortConverterArithmetic<unsigned short>
{};
template <>
class PortConverter<int> : public PortConverterArithmetic<int>
{};
template <>
class PortConverter<unsigned int> : public PortConverterArithmetic<unsigned int>
{};
template <>
class PortConverter<long> : public PortConverterArithmetic<long>
{};
template <>
class PortConverter<unsigned long> : public PortConverterArithmetic<unsigned long>
{};
template <>
class PortConverter<long long> : public PortConverterArithmetic<long long>
{};
template <>
class PortConverter<unsigned long long> : public PortConverterArithmetic<unsigned long long>
{};
template <>
class PortConverter<float> : public PortConverterArithmetic<float>
{};
template <>
class PortConverter<double> : public PortConverterArithmetic<double>
{};
template <>
class PortConverter<long double> : public PortConverterArithmetic<long double>
{};



template <class T, int Dim, bool Packed>
class PortConverter<Vector<T, Dim, Packed>> : public PortConverterCollection<Vector<T, Dim, Packed>> {
	using VectorT = Vector<T, Dim, Packed>;
public:
	PortConverter() : PortConverterCollection<Vector<T, Dim, Packed>>(
		&FromVector<char, true>,
		&FromVector<unsigned char, true>,
		&FromVector<signed char, true>,
		&FromVector<short, true>,
		&FromVector<unsigned short, true>,
		&FromVector<int, true>,
		&FromVector<unsigned int, true>,
		&FromVector<long, true>,
		&FromVector<unsigned long, true>,
		&FromVector<long long, true>,
		&FromVector<unsigned long long, true>,
		&FromVector<float, true>,
		&FromVector<double, true>,
		&FromVector<long double, true>,

		&FromVector<char, false>,
		&FromVector<unsigned char, false>,
		&FromVector<signed char, false>,
		&FromVector<short, false>,
		&FromVector<unsigned short, false>,
		&FromVector<int, false>,
		&FromVector<unsigned int, false>,
		&FromVector<long, false>,
		&FromVector<unsigned long, false>,
		&FromVector<long long, false>,
		&FromVector<unsigned long long, false>,
		&FromVector<float, false>,
		&FromVector<double, false>,
		&FromVector<long double, false>,

		&FromString)
	{}

protected:
	template <class T2, bool Packed2>
	static VectorT FromVector(const Vector<T2, Dim, Packed2>& src) {
		src.Length();
		return VectorT(src);
	}

	static VectorT FromString(const std::string& str) {
		const char* end;
		auto v = strtovec<VectorT>(str.c_str(), &end);
		if (str.c_str() == end) {
			throw InvalidCastException("Invalid vector format.");
		}
		return v;
	}
};



} // namespace inl