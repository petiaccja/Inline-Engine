#pragma once

#include "Port.hpp"
#include <type_traits>
#include <cstdlib>
#include <string>


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


	static long long FromStringHelper(const std::string& str, std::true_type, std::true_type) {
		if (str.empty()) {
			throw InvalidCastException("Cannot convert empty string to artihmetic.");
		}
		char* pend;
		long long value = strtoll(str.c_str(), &pend, 10);
		if (*pend != '\0') {
			throw InvalidCastException("Invalid number format.");
		}
		return value;
	}

	static unsigned long long FromStringHelper(const std::string& str, std::true_type, std::false_type) {
		if (str.empty()) {
			throw InvalidCastException("Cannot convert empty string to artihmetic.");
		}
		char* pend;
		unsigned long long value = strtoull(str.c_str(), &pend, 10);
		if (*pend != '\0') {
			throw InvalidCastException("Invalid number format.");
		}
		return value;
	}
	static long double FromStringHelper(const std::string& str, std::false_type, std::true_type) {
		if (str.empty()) {
			throw InvalidCastException("Cannot convert empty string to artihmetic.");
		}
		char* pend;
		long double value = strtold(str.c_str(), &pend);
		if (*pend != '\0') {
			throw InvalidCastException("Invalid number format.");
		}
		return value;
	}

	static AritT FromString(const std::string& str) {
		return (AritT)FromStringHelper(
			str,
			std::integral_constant<bool, std::is_integral<AritT>::value>(),
			std::integral_constant<bool, std::is_signed<AritT>::value>());
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



} // namespace inl