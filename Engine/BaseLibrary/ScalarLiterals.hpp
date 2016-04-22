#pragma once

////////////////////////////////
// SI UNITS

/// nano units
constexpr long double operator "" _nano(long double n) {
	return n * 0.001_micro;
}


/// micro units
constexpr long double operator "" _micro(long double n) {
	return n * 0.001_mili;
}


/// mili units
constexpr long double operator "" _mili(long double n) {
	return n * 0.001l;
}


/// kilo units
constexpr unsigned long long operator "" _kilo(unsigned long long n) {
	return n * 1000;
}
constexpr long double operator "" _kilo(long double n) {
	return n * 1000.l;
}


/// mega units
constexpr unsigned long long operator "" _mega(unsigned long long n) {
	return n * 1000_kilo;
}
constexpr long double operator "" _mega(long double n) {
	return n * 1000._kilo;
}


/// giga units
constexpr unsigned long long operator "" _giga(unsigned long long n) {
	return n * 1000_mega;
}
constexpr long double operator "" _giga(long double n) {
	return n * 1000._mega;
}


/// tera units
constexpr unsigned long long operator "" _tera(unsigned long long n) {
	return n * 1000_giga;
}
constexpr long double operator "" _tera(long double n) {
	return n * 1000._giga;
}





///////////////////////////////////////
// BINARY UNITS

/// kibi units
constexpr unsigned long long operator "" _Ki(unsigned long long n) {
	return n * 1024;
}


/// mebi units
constexpr unsigned long long operator "" _Mi(unsigned long long n) {
	return n * 1024_Ki;
}


/// gibi units
constexpr unsigned long long operator "" _Gi(unsigned long long n) {
	return n * 1024_Mi;
}


/// tebi units
constexpr unsigned long long operator "" _Ti(unsigned long long n) {
	return n * 1024_Gi;
}
