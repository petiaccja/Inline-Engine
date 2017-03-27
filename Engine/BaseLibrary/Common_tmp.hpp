#pragma once
#pragma once
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

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

	void Zero()
	{
		x = 0;
		y = 0;
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

	vec2 operator - (const vec2& b)
	{
		return vec2(x - b.x, y - b.y);
	}
};

class Color
{
public:
	Color() {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): r(r), g(g), b(b), a(a) {}
	Color(uint8_t r, uint8_t g, uint8_t b): Color(r, g, b, 255) {}
	Color(uint8_t greyscale) : Color(greyscale, greyscale, greyscale, 255) {}

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

	Rect(const T& x, const T& y, const T& width, const T& height)
		:x(x), y(y), width(width), height(height) {
	}

	bool IsPointInside(ivec2 point)
	{
		return	point.x >= x && point.x <= x + width &&
				point.y >= y && point.y <= y + height;
	}

	vec2 GetCenter()
	{
		return vec2(x + 0.5f * width, y + 0.5f * height);
	}

	float GetRight()
	{
		return x + width;
	}

	float GetLeft()
	{
		return x;
	}

	float GetTop()
	{
		return y;
	}

	float GetBottom()
	{
		return y + height;
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




template<class T>
T saturate(const T& val)
{
	T result = val;

	if (result < 0)
		result = 0;

	if (result > 1)
		result = 1;

	return result;
}


///////////////////////////////////////////////////////////////////////////
////////////////////////// DELEGATE, TODO REVIEW //////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename Signature>
struct Delegate;

template <typename... Args>
struct Delegate<void(Args...)>
{
	struct base {
		virtual ~base() {}
		//virtual bool do_cmp(base* other) = 0;
		virtual void do_call(Args... args) = 0;
	};
	template <typename T>
	struct call : base {
		T d_callback;
		template <typename S>
		call(S&& callback) : d_callback(std::forward<S>(callback)) {}

		//bool do_cmp(base* other) {
		//	call<T>* tmp = dynamic_cast<call<T>*>(other);
		//	return tmp && this->d_callback == tmp->d_callback;
		//}
		void do_call(Args... args) {
			return this->d_callback(std::forward<Args>(args)...);
		}
	};
	std::vector<std::shared_ptr<base>> d_callbacks;

	Delegate(Delegate const& other)
	{
		*this = other;
	}

	Delegate& operator=(Delegate const& other)
	{
		d_callbacks = other.d_callbacks;
		return *this;
	}

	operator bool() const
	{
		return d_callbacks.size() != 0;
	}

public:
	Delegate() {}
	template <typename T>
	Delegate& operator+= (T&& callback) {
		this->d_callbacks.emplace_back(new call<T>(std::forward<T>(callback)));
		return *this;
	}
	//template <typename T>
	//Delegate& operator-= (T&& callback) {
	//	call<T> tmp(std::forward<T>(callback));
	//
	//	auto it = std::remove_if(d_callbacks.begin(), d_callbacks.end(), [&](std::unique_ptr<base>& other)
	//	{
	//		return tmp.do_cmp(other.get());
	//	});
	//	 
	//	this->d_callbacks.erase(it, this->d_callbacks.end());
	//	return *this;
	//}

	void operator()(Args... args)
	{
		// I'm sorry but it's necessary to copy the array, because callbacks can remove and add elements to them lol
		auto callbacks = d_callbacks;
		for (auto& callback : callbacks) {
			callback->do_call(args...);
		}
	}
};

template <typename RC, typename Class, typename... Args>
class MemberCall_{
	Class* d_object;
	RC(Class::*d_member)(Args...);
public:
	MemberCall_(Class* object, RC(Class::*member)(Args...))
		: d_object(object)
		, d_member(member) {
	}
	RC operator()(Args... args) {
		return (this->d_object->*this->d_member)(std::forward<Args>(args)...);
	}
	bool operator== (MemberCall_ const& other) const {
		return this->d_object == other.d_object
			&& this->d_member == other.d_member;
	}
	bool operator!= (MemberCall_ const& other) const {
		return !(*this == other);
	}
};

template <typename RC, typename Class, typename... Args>
MemberCall_<RC, Class, Args...> MemberCall(Class& object,
	RC(Class::*member)(Args...)) {
	return MemberCall<RC, Class, Args...>(&object, member);
}