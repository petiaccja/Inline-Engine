#pragma once

namespace inl::prefix {


////////////////////////////////
// SI UNITS

constexpr long double operator "" _milli(long double x) {
	return x * 0.001l;
}
constexpr long double operator "" _milli(unsigned long long x) {
	return x * 0.001l;
}


constexpr long double operator "" _micro(long double x) {
	return x * 0.001_milli;
}
constexpr long double operator "" _micro(unsigned long long x) {
	return x * 0.001_milli;
}


constexpr long double operator "" _nano(long double x) {
	return x * 0.001_micro;
}
constexpr long double operator "" _nano(unsigned long long x) {
	return x * 0.001_micro;
}


constexpr long double operator "" _pico(long double x) {
	return x * 0.001_nano;
}
constexpr long double operator "" _pico(unsigned long long x) {
	return x * 0.001_nano;
}




constexpr long long operator "" _kilo(unsigned long long x) {
	return x * 1000;
}
constexpr long double operator "" _kilo(long double x) {
	return x * 1000.l;
}


constexpr long long operator "" _mega(unsigned long long x) {
	return x * 1000_kilo;
}
constexpr long double operator "" _mega(long double x) {
	return x * 1000._kilo;
}


constexpr long long operator "" _giga(unsigned long long x) {
	return x * 1000_mega;
}
constexpr long double operator "" _giga(long double x) {
	return x * 1000._mega;
}


constexpr long long operator "" _tera(unsigned long long x) {
	return x * 1000_giga;
}
constexpr long double operator "" _tera(long double x) {
	return x * 1000._giga;
}





///////////////////////////////////////
// BINARY UNITS


constexpr long long operator "" _Ki(unsigned long long x) {
	return x * 1024;
}


constexpr long long operator "" _Mi(unsigned long long x) {
	return x * 1024_Ki;
}


constexpr long long operator "" _Gi(unsigned long long x) {
	return x * 1024_Mi;
}


constexpr long long operator "" _Ti(unsigned long long x) {
	return x * 1024_Gi;
}


}
