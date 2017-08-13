#pragma once
#pragma once
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

//#include <mathfu/mathfu_exc.hpp>
//using namespace mathfu;

#include <InlineMath.hpp>
using namespace inl;

struct Ray
{
	Vec3 origin;
	Vec3 direction;
};

class Color
{
public:
	inline Color() {}
	inline Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): r(r), g(g), b(b), a(a) {}
	inline Color(uint8_t r, uint8_t g, uint8_t b): Color(r, g, b, 255) {}
	inline Color(uint8_t greyscale) : Color(greyscale, greyscale, greyscale, 255) {}

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	inline Color operator + (int val) const
	{
		Color color;
		color.r = r + val;
		color.g = g + val;
		color.b = b + val;

		return color;
	}

	static Color Random(bool bRandomAlpha = false)
	{
		Color color;
		color.r = std::rand() % 256;
		color.g = std::rand() % 256;
		color.b = std::rand() % 256;

		if (bRandomAlpha)
			color.a = std::rand() % 256;
		else
			color.a = 255;

		return color;
	}

	inline bool operator == (const Color& color) const
	{
		return memcmp(this, &color, sizeof(Color)) == 0;
	}

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

	Vec2 bottomLeftPercentNormed;
	Vec2 topRightPercentNormed;
};

template<class T>
class Rect
{
public:
	inline Rect():left(0), right(0), top(0), bottom(0)
	{}

	inline Rect(const Vec2& pos, const Vec2& size)
	: left(pos.x), right(pos.x + size.x), top(pos.y), bottom(pos.y + size.y)
	{}

	inline Rect(const T& left, const T& right, const T& top, const T& bottom)
	:left(left), right(right), top(top), bottom(bottom)
	{}
	
	static Rect FromSize(float left, float top, float width, float height)
	{
		return Rect(left, top, left + width, top + height);
	}

	void MoveSides(const Rect& offset)
	{
		left   += offset.left;
		right  += offset.right;
		top	   += offset.top;
		bottom += offset.bottom;
	}

	void MoveSidesLocal(const Rect& offset)
	{
		left   -= offset.left;
		right  += offset.right;
		top	   -= offset.top;
		bottom += offset.bottom;
	}

	void Union(const Rect& other)
	{
		*this = Rect::Union(*this, other);
	};

	void UnionWidth(float width)
	{
		Rect selfCopy = *this;
		selfCopy.SetWidth(width);

		Union(*this, selfCopy);
	}

	void UnionHeight(float height)
	{
		Rect selfCopy = *this;
		selfCopy.SetHeight(height);

		Union(*this, selfCopy);
	}

	static Rect Union(const Rect& a, const Rect& b)
	{
		Rect result;

		float maxLeft = std::min(a.left, b.left);
		float maxTop = std::min(a.top, b.top);
		float minBottom = std::max(a.bottom, b.bottom);
		float minRight = std::max(a.right, b.right);

		result.left = maxLeft;
		result.top = maxTop;
		result.right = minRight;
		result.bottom = minBottom;

		return result;
	};

	void Intersect(const Rect& other)
	{
		*this = Rect::Intersect(*this, other);
	}

	static Rect Intersect(const Rect& a, const Rect& b)
	{
		Rect result;

		float maxLeft = std::max(a.left, b.left);
		float maxTop = std::max(a.top, b.top);
		float minBottom = std::min(a.bottom, b.bottom);
		float minRight = std::min(a.right, b.right);

		result.left = maxLeft;
		result.top = maxTop;
		result.right = minRight;
		result.bottom = minBottom;

		return result;
	}

	Rect operator -() const
	{
		Rect result;
		result.left = -left;
		result.right = -right;
		result.top = -top;
		result.bottom = -bottom;
		return result;
	};

	Rect operator - (const Rect& other) const
	{
		Rect result;
		result.left = left - other.top;
		result.right = right - other.right;
		result.top = top - other.top;
		result.bottom = bottom - other.bottom;
		return result;
	}

	bool IsPointInside(Vec2 point) const
	{
		return	point.x >= left && point.x <= right	&&
				point.y >= top  && point.y <= bottom;
	}

	bool IsRectInside(const Rect<T>& other)
	{
		return	other.left >= left && other.right <= right && other.top >= top && other.bottom <= bottom;
	}

	Vec2 GetSize() const { return Vec2(GetWidth(), GetHeight()); }
	Vec2 GetPos() const { return Vec2(left, top); }
	float GetPosX() const { return GetPos().x; }
	float GetPosY() const { return GetPos().y; }

	float GetSizeX() const { return GetSize().x; }
	float GetSizeY() const { return GetSize().y; }

	Vec2 GetCenter() const
	{
		return Vec2(left + 0.5f * GetWidth(), top + 0.5f * GetHeight());
	}

	bool operator == (const Rect<T>& other) const
	{
		return memcmp(this, &other, sizeof(Rect<T>)) == 0;
	}

	bool operator != (const Rect<T>& other) const
	{
		return memcmp(this, &other, sizeof(Rect<T>)) != 0;
	}

	void SetWidth(float width)
	{
		right = left + width;
	}

	void SetHeight(float height)
	{
		bottom = top + height;
	}

	float GetWidth() const
	{
		return right - left;
	}

	float GetHeight() const
	{
		return bottom - top;
	}

	void SetSize(const Vec2& size)
	{
		SetWidth(size.x);
		SetHeight(size.y);
	}

	T left;
	T right;
	T top;
	T bottom;
};

typedef Rect<float> RectF;







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




template<class T>
T Clamp01(const T& val)
{
	T result = val;

	if (result < 0)
		result = 0;

	if (result > 1)
		result = 1;

	return result;
}

template<class T>
T Clamp(const T& val, const T& minVal, const T& maxVal)
{
	T result = val;

	if (result < minVal)
		result = minVal;

	if (result > maxVal)
		result = maxVal;

	return result;
}