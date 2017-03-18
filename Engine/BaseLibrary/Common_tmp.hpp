#pragma once
#pragma once
#include <string>

//#undef min
//#undef max

struct ivec2
{
	inline ivec2() {}
	ivec2(int x, int y): x(x), y(y) {}

	ivec2& operator +=(const ivec2& rhs)
	{
		x += rhs.x;
		x += rhs.y;
		return *this;
	}

	int x;
	int y;
};

struct uvec2
{
	inline uvec2() {}
	uvec2(unsigned int x, unsigned int y): x(x), y(y) {}
	unsigned int x;
	unsigned int y;
};

struct vec2
{
	inline vec2() {}
	vec2(float x, float y): x(x), y(y) {}

	float x;
	float y;
};

class Color
{
public:
	Color() {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): r(r), g(g), b(b), a(a) {}
	Color(uint8_t r, uint8_t g, uint8_t b): Color(r, g, b, 255) {}

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

public:
	static Color BLACK;
	static Color WHITE;
	static Color RED;
	static Color GREEN;
	static Color BLUE;
};

struct RectNormed
{
	RectNormed() :bottomLeftPercentNormed(0, 0), topRightPercentNormed(1, 1) {}

	vec2 bottomLeftPercentNormed;
	vec2 topRightPercentNormed;
};

template<class T>
class Rect
{
public:
	Rect() {
	}

	Rect(T& x, T& y, T& width, T& height)
		:x(x), y(y), width(width), height(height) {
	}

	vec2 GetCenter()
	{
		return vec2(x + 0.5f * width, y + 0.5f * height);
	}

	T x;
	T y;
	T width;
	T height;
};

#define ENUM_CLASS_BITFLAG( enumClass, enumType )  \
enum class enumClass : enumType; \
class zzzEnum_##enumClass\
{ \
public:\
	zzzEnum_##enumClass(enumClass e) :flag(e) {}\
	operator enumClass() { return flag; }\
	operator bool() { return (enumType)flag != 0; }\
public:\
	enumClass flag;\
};\
inline zzzEnum_##enumClass operator | (enumClass a, enumClass b) \
{\
	return (enumClass)((enumType)a | (enumType)b);\
}\
inline bool operator & (enumClass a, enumClass b)\
{\
	return ((enumType)a & (enumType)b) > 0;\
}\
inline zzzEnum_##enumClass operator ~ (zzzEnum_##enumClass a)\
{\
	return (enumClass)(~(enumType)a.flag);\
}\
inline zzzEnum_##enumClass operator ^ (enumClass a, enumClass b)\
{\
	return (enumClass)((enumType)a ^ (enumType)b);\
}\
inline zzzEnum_##enumClass operator >> (enumClass a, enumClass b)\
{\
	return (enumClass)((enumType)a >> (enumType)b);\
}\
inline zzzEnum_##enumClass operator << (enumClass a, enumClass b)\
{\
	return (enumClass)((enumType)a << (enumType)b);\
}\
inline enumClass& operator |= (enumClass& a, enumClass b)\
{\
	(enumType&)a |= (enumType&)b; \
	return a; \
}\
inline enumClass& operator &= (enumClass& a, enumClass b)\
{\
	(enumType&)a &= (enumType&)b; \
	return a; \
}\
inline enumClass& operator ^= (enumClass& a, enumClass b)\
{\
	(enumType&)a ^= (enumType&)b; \
	return a; \
}\
enum class enumClass : enumType