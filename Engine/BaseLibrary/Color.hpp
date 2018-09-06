#pragma once

#include <InlineMath.hpp>
#include <iostream>


namespace inl {


template <class Scalar>
class Color {
public:
	union {
		struct {
			Scalar r, g, b, a;
		};
		Vector<Scalar, 4, false> v;
	};

public:
	Color() {}

	Color(Scalar r, Scalar g, Scalar b, Scalar a = Scalar(1)) : r(r), g(g), b(b), a(a) {}	

	Color(const Color& rhs) {
		v = rhs.v;
	}

	template <class S2>
	Color(const Color<S2>& rhs) {
		Vector<long double, 4, false> transition;

		// signed integer source
		if (std::is_integral<S2>::value && std::is_signed<S2>::value) {
			long double minval = std::numeric_limits<S2>::min();
			long double maxval = std::numeric_limits<S2>::max();
			long double range = maxval - minval;
			transition = rhs.v;
			transition -= minval;
			transition /= range;
		}
		// unsigned integer source
		else if (std::is_integral<S2>::value && !std::is_signed<S2>::value) {
			long double minval = std::numeric_limits<S2>::min(); // 0
			long double maxval = std::numeric_limits<S2>::max();
			long double range = maxval - minval;
			transition = rhs.v;
			transition -= minval;
			transition /= range;
		}
		// floating point source
		else {
			transition = rhs.v;
		}


		// signed integer dst
		if (std::is_integral<Scalar>::value && std::is_signed<Scalar>::value) {
			long double minval = std::numeric_limits<Scalar>::min();
			long double maxval = std::numeric_limits<Scalar>::max();
			long double range = maxval - minval;
			transition = Saturate(transition);
			transition *= range;
			transition -= minval;
			transition += 0.5;
			v = transition;
		}
		// unsigned integer dst
		else if (std::is_integral<Scalar>::value && !std::is_signed<Scalar>::value) {
			long double minval = std::numeric_limits<Scalar>::min(); // 0
			long double maxval = std::numeric_limits<Scalar>::max();
			long double range = maxval - minval;
			transition = Saturate(transition);
			transition *= range;
			transition += 0.5;
			v = transition;
		}
		// floating point dst
		else {
			v = transition;
		}
	}

	Color& operator=(const Color& rhs) {
		v = rhs.v;
		return *this;
	}

	template <class S2>
	Color& operator=(const Color<S2>& rhs) {
		*this = Color(rhs);
		return *this;
	}


	Color& operator*=(const Color& rhs) {
		v *= rhs.v;
		return *this;
	}
	Color& operator/=(const Color& rhs) {
		v /= rhs.v;
		return *this;
	}
	Color& operator+=(const Color& rhs) {
		v += rhs.v;
		return *this;
	}
	Color& operator-=(const Color& rhs) {
		v -= rhs.v;
		return *this;
	}

	Color operator*(const Color& rhs) const {
		return Color(*this) *= rhs;
	}
	Color operator/(const Color& rhs) const {
		return Color(*this) /= rhs;
	}
	Color operator+(const Color& rhs) const {
		return Color(*this) += rhs;
	}
	Color operator-(const Color& rhs) const {
		return Color(*this) -= rhs;
	}

	bool operator ==(const Color& rhs) const {
		return v == rhs.v;
	}

	static Scalar Dot(const Color& lhs, const Color& rhs) {
		return Dot(lhs.v, rhs.v);
	}

	Scalar Greyscale() const {
		return Dot(v, Color(Color<float>(0.2126f, 0.7152f, 0.0722f, 0.0f)).v);
	}
};


template <class T>
std::ostream& operator<<(std::ostream& os, const Color<T>& obj) {
	os << obj.v;
	return os;
}

template <class T>
std::istream& operator<<(std::istream& is, Color<T>& obj) {
	is >> obj.v;
	return is;
}


using ColorF = Color<float>;
using ColorI = Color<uint8_t>;
using ColorI16 = Color<uint16_t>;
using ColorI32 = Color<uint32_t>;

} // namespace inl
