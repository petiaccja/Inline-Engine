#pragma once

#include <InlineMath.hpp>


namespace inl {


template <class T>
class Rect {
public:
	using Vec2T = Vector<T, 2, false>;
public:
	Rect() : left(0), right(0), bottom(0), top(0) {}
	Rect(T left, T right, T bottom, T top)
		: left(left), right(right), bottom(bottom), top(top) {}
	Rect(Vec2T anchorPoint1, Vec2T anchorPoint2);
	static Rect FromSize(T bottom, T left, T width, T height);
	static Rect FromSize(Vec2T bottomLeft, Vec2T size);

	Vec2T GetSize() const;
	T GetWidth() const;
	T GetHeight() const;

	T GetArea() const;

	Vec2T GetBottomLeft() const;
	Vec2T GetTopLeft() const;
	Vec2T GetBottomRight() const;
	Vec2T GetTopRight() const;
	Vec2T GetCenter() const;

	void SetSize(Vec2T newSize, Vec2T origin = { 0.5f, 0.5f });
	void SetWidth(T newWidth, T origin = 0.5f);
	void SetHeight(T newHeight, T origin = 0.5f);

	void Move(const Vec2T& offset);
	void MoveSides(const Rect& offset);

	static Rect Union(const Rect& lhs, const Rect& rhs);
	static Rect Intersection(const Rect& lhs, const Rect& rhs);
	void Union(const Rect& rhs) { *this = Union(*this, rhs); }
	void Interect(const Rect& rhs) { *this = Intersection(*this, rhs); }

	bool IsPointInside(const Vec2T& arg) const;
	bool IsRectInside(const Rect& arg) const;
	bool IsIntersecting(const Rect& arg) const;

	bool operator==(const Rect& arg) const;
	bool operator!=(const Rect& arg) const;
public:
	T left, right, bottom, top;
};


template <class T>
Rect<T>::Rect(Vec2T anchorPoint1, Vec2T anchorPoint2) {
	Vec2T minp = Min(anchorPoint1, anchorPoint2);
	Vec2T maxp = Max(anchorPoint1, anchorPoint2);
	left = minp.x;
	right = maxp.x;
	bottom = minp.y;
	top = maxp.y;
}

template <class T>
Rect<T> Rect<T>::FromSize(T bottom, T left, T width, T height) {
	return FromSize({ left, bottom }, { width, height });
}

template <class T>
Rect<T> Rect<T>::FromSize(Vec2T bottomLeft, Vec2T size) {
	size += bottomLeft;
	left = bottomLeft.x;
	bottom = bottomLeft.y;
	right = size.x;
	top = size.y;
}

template <class T>
typename Rect<T>::Vec2T Rect<T>::GetSize() const {
	return { right - left, top - bottom };
}

template <class T>
T Rect<T>::GetWidth() const {
	return right - left;
}

template <class T>
T Rect<T>::GetHeight() const {
	return top - bottom;
}

template <class T>
T Rect<T>::GetArea() const {
	auto s = GetSize();
	return s.x * s.y;
}

template <class T>
typename Rect<T>::Vec2T Rect<T>::GetBottomLeft() const {
	return { left, bottom };
}

template <class T>
typename Rect<T>::Vec2T Rect<T>::GetTopLeft() const {
	return { left, top };
}

template <class T>
typename Rect<T>::Vec2T Rect<T>::GetBottomRight() const {
	return { right, bottom };
}

template <class T>
typename Rect<T>::Vec2T Rect<T>::GetTopRight() const {
	return { right, top };
}

template <class T>
typename Rect<T>::Vec2T Rect<T>::GetCenter() const {
	return (GetBottomLeft() + GetTopRight()) * T(0.5);
}


template <class T>
void Rect<T>::SetSize(Vec2T newSize, Vec2T origin) {
	SetWidth(newSize.x, origin.x);
	SetHeight(newSize.y, origin.y);
}

template <class T>
void Rect<T>::SetWidth(T newWidth, T origin) {
	T p = (T(1) - origin)*left + origin*right;
	left = p + (T(1) - origin)*newWidth;
	right = p + origin*newWidth;
}

template <class T>
void Rect<T>::SetHeight(T newHeight, T origin) {
	T p = (T(1) - origin)*bottom + origin*top;
	bottom  = p + (T(1) - origin)*newHeight;
	top = p + origin*newHeight;
}


template <class T>
void Rect<T>::Move(const Vec2T& offset) {
	left += offset.x;
	right += offset.x;
	bottom += offset.y;
	top += offset.y;
}

template <class T>
void Rect<T>::MoveSides(const Rect& offset) {
	left += offset.left;
	right += offset.right;
	bottom += offset.bottom;
	top += offset.top;
}


template <class T>
Rect<T> Rect<T>::Union(const Rect& lhs, const Rect& rhs) {
	Vec2T min1 = Min(lhs.GetBottomLeft(), lhs.GetTopRight());
	Vec2T max1 = Max(lhs.GetBottomLeft(), lhs.GetTopRight());
	Vec2T min2 = Min(rhs.GetBottomLeft(), rhs.GetTopRight());
	Vec2T max2 = Max(rhs.GetBottomLeft(), rhs.GetTopRight());
	
	Vec2T minp = Min(min1, min2);
	Vec2T maxp = Max(max1, max2);

	return Rect(minp, maxp);
}

template <class T>
Rect<T> Rect<T>::Intersection(const Rect& lhs, const Rect& rhs) {
	Vec2T min1 = Min(lhs.GetBottomLeft(), lhs.GetTopRight());
	Vec2T max1 = Max(lhs.GetBottomLeft(), lhs.GetTopRight());
	Vec2T min2 = Min(rhs.GetBottomLeft(), rhs.GetTopRight());
	Vec2T max2 = Max(rhs.GetBottomLeft(), rhs.GetTopRight());

	Vec2T minp = Max(min1, min2);
	Vec2T maxp = Min(max1, max2);

	if (minp.x > maxp.x || minp.y > maxp.y) {
		return Rect(0, 0, 0, 0);
	}

	return Rect(minp, maxp);
}


template <class T>
bool Rect<T>::IsPointInside(const Vec2T& arg) const {
	return	arg.x >= left && arg.x <= right	&&
		arg.y >= top  && arg.y <= bottom;
}

template <class T>
bool Rect<T>::IsRectInside(const Rect& arg) const {
	return	arg.left >= left && arg.right <= right && arg.top >= top && arg.bottom <= bottom;
}

template <class T>
bool Rect<T>::IsIntersecting(const Rect& arg) const {
	Rect intersection = Intersection(*this, arg);
	return intersection.GetArea() != 0;
}


template <class T>
bool Rect<T>::operator==(const Rect& rhs) const {
	return left == rhs.left && right == rhs.right && top == rhs.top && bottom == rhs.bottom;
}

template <class T>
bool Rect<T>::operator!=(const Rect& rhs) const {
	return !(*this == rhs);
}


using RectF = Rect<float>;
using RectI = Rect<int>;



} // namespace inl
