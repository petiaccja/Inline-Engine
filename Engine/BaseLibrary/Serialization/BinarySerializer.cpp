#include "BinarySerializer.hpp"


namespace inl {


void BinarySerializer::Insert(const_iterator where, uint8_t value) {
	auto it = where == end() ? buffer.end() : buffer.begin() + where.index;
	buffer.insert(it, value);
}

void BinarySerializer::Insert(const_iterator where, uint8_t* data, size_t size) {
	Insert(where, data, data + size);
}


void BinarySerializer::PushFront(uint8_t* data, size_t size) {
	PushFront(data, data + size);
}

void BinarySerializer::PushFront(uint8_t value) {
	buffer.push_front(value);
}


void BinarySerializer::PushBack(uint8_t* data, size_t size) {
	PushBack(data, data + size);
}

void BinarySerializer::PushBack(uint8_t value) {
	buffer.push_back(value);
}

uint8_t BinarySerializer::PopFront() {
	uint8_t value = *buffer.begin();
	buffer.pop_front();
	return value;
}

uint8_t BinarySerializer::PopBack() {
	uint8_t value = *--buffer.end();
	buffer.pop_back();
	return value;
}

void BinarySerializer::Erase(const_iterator where, size_t size) {
	Erase(where, where + size);
}

void BinarySerializer::Erase(const_iterator first, const_iterator last) {
	auto it_first = buffer.begin() + first.index;
	auto it_last = last == end() ? buffer.end() : buffer.begin() + last.index;
	buffer.erase(it_first, it_last);
}



uint8_t& BinarySerializer::operator[](size_t index) {
	return buffer[index];
}

const uint8_t& BinarySerializer::operator[](size_t index) const {
	return buffer[index];
}



BinarySerializer::iterator BinarySerializer::begin() {
	iterator it;
	it.index = 0;
	it.parent = this;
	return it;
}

BinarySerializer::iterator BinarySerializer::end() {
	iterator it;
	it.index = BeginPosition();
	it.parent = this;
	return it;
}

BinarySerializer::const_iterator BinarySerializer::begin() const {
	const_iterator it;
	it.index = 0;
	it.parent = this;
	return it;
}

BinarySerializer::const_iterator BinarySerializer::end() const {
	const_iterator it;
	it.index = BeginPosition();
	it.parent = this;
	return it;
}

BinarySerializer::const_iterator BinarySerializer::cbegin() const {
	return begin();
}

BinarySerializer::const_iterator BinarySerializer::cend() const {
	return end();
}



uint32_t FloatToIEEE754(float v) {
	// check if platform is using IEEE-754 floats
	// msvc can't compile to anything else anyways so...
#if defined(__STDC_IEC_559__ ) || defined(_MSC_VER)
	return *reinterpret_cast<uint32_t*>(&v);
#else
	static_assert(false, "I'm done with this shit, write you own...");
	float significand;
	int exponent;
	bool isNegative = false;
	uint32_t bits = 0;

	significand = std::frexp(v, &exponent);
	if (significand < 0.0f) {
		isNegative = true;
		significand = -significand;
	}

	exponent += 127 - 1;
	significand *= 2;

	unsigned significand_int = unsigned((significand - 1)*(1u << 23));

	bits |= (1 << 31) * isNegative;
	bits |= (unsigned)exponent << 23;
	bits |= ((1u << 24) - 1) & significand_int;

	return bits;
#endif
}

float IEEE754ToFloat(uint32_t b) {
#if defined(__STDC_IEC_559__ ) || defined(_MSC_VER)
	return *reinterpret_cast<float*>(&b);
#else
	static_assert(false, "I'm done with this shit, write you own...");
	int signmult = (b & (1 << 31)) > 0 ? -1 : 1;
	float significand;
	int exp;

	significand = 1 + pow(2.0f, -23) * (b & ((1u << 24) - 1));
	exp = int(0xFFu & (b >> 23)) - 127;

	return signmult * significand * pow(2.0f, exp);
#endif
}

uint64_t DoubleToIEEE754(double v) {
#if defined(__STDC_IEC_559__ ) || defined(_MSC_VER)
	return *reinterpret_cast<uint64_t*>(&v);
#else
	static_assert(false, "I'm done with this shit, write you own...");
	double significand;
	int exponent;
	bool isNegative = false;
	uint64_t bits = 0;

	significand = std::frexp(v, &exponent);
	if (significand < 0.0) {
		isNegative = true;
		significand = -significand;
	}

	bits |= (1ull << 63) * isNegative;
	bits |= uint64_t(exponent + 1023) << 62;
	bits |= ((1ull << 53) - 1) & uint64_t((significand - 1.0f)*(1ull << 52));

	return bits;
#endif
}

double IEEE754ToDouble(uint64_t b) {
#if defined(__STDC_IEC_559__ ) || defined(_MSC_VER)
	return *reinterpret_cast<double*>(&b);
#else
	static_assert(false, "I'm done with this shit, write you own...");
	int signmult = (b & (1ull << 63)) > 0 ? -1 : 1;
	float significand;
	int exp;

	significand = 1 + pow(2.0, -52) * (b & ((1ull << 53) - 1));
	exp = int(0x7FFu & (b >> 52)) - 1023;

	return signmult * significand * pow(2.0, exp);
#endif
}



BinarySerializer& operator << (BinarySerializer& s, float v) {
	uint32_t ieee754 = FloatToIEEE754(v);
	s << ieee754;
	return s;
}

BinarySerializer& operator << (BinarySerializer& s, double v) {
	uint64_t ieee754 = DoubleToIEEE754(v);
	s << ieee754;
	return s;
}


BinarySerializer& operator >> (BinarySerializer& s, float& v) {
	uint32_t ieee754;
	s >> ieee754;
	v = IEEE754ToFloat(ieee754);
	return s;
}

BinarySerializer& operator >> (BinarySerializer& s, double& v) {
	uint64_t ieee754;
	s >> ieee754;
	v = IEEE754ToDouble(ieee754);
	return s;
}

BinarySerializer& operator << (float& v, BinarySerializer& s) {
	uint32_t ieee754;
	ieee754 << s;
	v = IEEE754ToFloat(ieee754);
	return s;
}

BinarySerializer& operator << (double& v, BinarySerializer& s) {
	uint64_t ieee754;
	ieee754 << s;
	v = IEEE754ToDouble(ieee754);
	return s;
}

BinarySerializer& operator >> (float v, BinarySerializer& s) {
	uint32_t ieee754 = FloatToIEEE754(v);
	ieee754 >> s;
	return s;
}

BinarySerializer& operator >> (double v, BinarySerializer& s) {
	uint64_t ieee754 = DoubleToIEEE754(v);
	ieee754 >> s;
	return s;
}


} // namespace inl
