#pragma once

#include <InlineMath.hpp>
#include <algorithm>

namespace inl {


template <class T, bool InvertHorizontal, bool InvertVertical>
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
	static Rect FromCenter(T centerx, T centery, T width, T height);
	static Rect FromCenter(Vec2T center, Vec2T size);

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
	void Intersect(const Rect& rhs) { *this = Intersection(*this, rhs); }

	bool IsPointInside(const Vec2T& arg) const;
	bool IsRectInside(const Rect& arg) const;
	bool IsIntersecting(const Rect& arg) const;

	bool operator==(const Rect& arg) const;
	bool operator!=(const Rect& arg) const;

public:
	T left, right, bottom, top;
};


template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical>::Rect(Vec2T anchorPoint1, Vec2T anchorPoint2) {
	Vec2T minp = Min(anchorPoint1, anchorPoint2);
	Vec2T maxp = Max(anchorPoint1, anchorPoint2);

	if constexpr (!InvertHorizontal) {
		left = minp.x;
		right = maxp.x;
	}
	else {
		right = minp.x;
		left = maxp.x;
	}

	if constexpr (!InvertVertical) {
		bottom = minp.y;
		top = maxp.y;
	}
	else {
		top = minp.y;
		bottom = maxp.y;
	}
}

template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical> Rect<T, InvertHorizontal, InvertVertical>::FromSize(T bottom, T left, T width, T height) {
	return FromSize({ left, bottom }, { width, height });
}

template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical> Rect<T, InvertHorizontal, InvertVertical>::FromCenter(T centerx, T centery, T width, T height) {
	return FromSize({ centerx, centery }, { width, height });
}

template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical> Rect<T, InvertHorizontal, InvertVertical>::FromSize(Vec2T bottomLeft, Vec2T size) {
	Rect rc;
	rc.left = bottomLeft.x;
	rc.bottom = bottomLeft.y;

	if constexpr (!InvertHorizontal) {
		rc.right = bottomLeft.x + size.x;
	}
	else {
		rc.right = bottomLeft.x - size.x;
	}

	if constexpr (!InvertVertical) {
		rc.top = bottomLeft.y + size.y;
	}
	else {
		rc.top = bottomLeft.y - size.y;
	}
	return rc;
}

template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical> Rect<T, InvertHorizontal, InvertVertical>::FromCenter(Vec2T center, Vec2T size) {
	Rect rc;

	if constexpr (!InvertHorizontal) {
		rc.right = center.x + size.x / T(2);
		rc.left = center.x - size.x / T(2);
	}
	else {
		rc.right = center.x - size.x / T(2);
		rc.left = center.x + size.x / T(2);
	}

	if constexpr (!InvertVertical) {
		rc.top = center.y + size.y / T(2);
		rc.bottom = center.y - size.y / T(2);
	}
	else {
		rc.top = center.y - size.y / T(2);
		rc.bottom = center.y + size.y / T(2);
	}
	return rc;
}

template <class T, bool InvertHorizontal, bool InvertVertical>
typename Rect<T, InvertHorizontal, InvertVertical>::Vec2T Rect<T, InvertHorizontal, InvertVertical>::GetSize() const {
	T width;
	T height;

	if constexpr (!InvertHorizontal) {
		width = right - left;
	}
	else {
		width = left - right;
	}

	if constexpr (!InvertVertical) {
		height = top - bottom;
	}
	else {
		height = bottom - top;
	}

	return { width, height };
}

template <class T, bool InvertHorizontal, bool InvertVertical>
T Rect<T, InvertHorizontal, InvertVertical>::GetWidth() const {

	if constexpr (!InvertHorizontal) {
		return right - left;
	}
	else {
		return left - right;
	}
}

template <class T, bool InvertHorizontal, bool InvertVertical>
T Rect<T, InvertHorizontal, InvertVertical>::GetHeight() const {
	if constexpr (!InvertVertical) {
		return top - bottom;
	}
	else {
		return bottom - top;
	}
}

template <class T, bool InvertHorizontal, bool InvertVertical>
T Rect<T, InvertHorizontal, InvertVertical>::GetArea() const {
	auto s = GetSize();
	return s.x * s.y;
}

template <class T, bool InvertHorizontal, bool InvertVertical>
typename Rect<T, InvertHorizontal, InvertVertical>::Vec2T Rect<T, InvertHorizontal, InvertVertical>::GetBottomLeft() const {
	return { left, bottom };
}

template <class T, bool InvertHorizontal, bool InvertVertical>
typename Rect<T, InvertHorizontal, InvertVertical>::Vec2T Rect<T, InvertHorizontal, InvertVertical>::GetTopLeft() const {
	return { left, top };
}

template <class T, bool InvertHorizontal, bool InvertVertical>
typename Rect<T, InvertHorizontal, InvertVertical>::Vec2T Rect<T, InvertHorizontal, InvertVertical>::GetBottomRight() const {
	return { right, bottom };
}

template <class T, bool InvertHorizontal, bool InvertVertical>
typename Rect<T, InvertHorizontal, InvertVertical>::Vec2T Rect<T, InvertHorizontal, InvertVertical>::GetTopRight() const {
	return { right, top };
}

template <class T, bool InvertHorizontal, bool InvertVertical>
typename Rect<T, InvertHorizontal, InvertVertical>::Vec2T Rect<T, InvertHorizontal, InvertVertical>::GetCenter() const {

	if constexpr (std::is_integral<T>::value) {
		if constexpr (sizeof(T) < 8) {
			return (GetBottomLeft() + GetTopRight()) * 0.5f;
		}
		else {
			return (GetBottomLeft() + GetTopRight()) * 0.5;
		}
	}
	else {
		return (GetBottomLeft() + GetTopRight()) * T(0.5);
	}
}

template <class T, bool InvertHorizontal, bool InvertVertical>
void Rect<T, InvertHorizontal, InvertVertical>::SetSize(Vec2T newSize, Vec2T origin) {
	SetWidth(newSize.x, origin.x);
	SetHeight(newSize.y, origin.y);
}

template <class T, bool InvertHorizontal, bool InvertVertical>
void Rect<T, InvertHorizontal, InvertVertical>::SetWidth(T newWidth, T origin) {
	T p = (T(1) - origin) * left + origin * right;

	if constexpr (!InvertHorizontal) {
		left = p - origin * newWidth;
		right = p + (T(1) - origin) * newWidth;
	}
	else {
		left = p + origin * newWidth;
		right = p - (T(1) - origin) * newWidth;
	}
}

template <class T, bool InvertHorizontal, bool InvertVertical>
void Rect<T, InvertHorizontal, InvertVertical>::SetHeight(T newHeight, T origin) {
	T p = (T(1) - origin) * bottom + origin * top;


	if constexpr (!InvertVertical) {
		bottom = p - origin * newHeight;
		top = p + (T(1) - origin) * newHeight;
	}
	else {
		bottom = p + origin * newHeight;
		top = p - (T(1) - origin) * newHeight;
	}
}


template <class T, bool InvertHorizontal, bool InvertVertical>
void Rect<T, InvertHorizontal, InvertVertical>::Move(const Vec2T& offset) {
	left += offset.x;
	right += offset.x;
	bottom += offset.y;
	top += offset.y;
}

template <class T, bool InvertHorizontal, bool InvertVertical>
void Rect<T, InvertHorizontal, InvertVertical>::MoveSides(const Rect& offset) {
	left += offset.left;
	right += offset.right;
	bottom += offset.bottom;
	top += offset.top;
}


template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical> Rect<T, InvertHorizontal, InvertVertical>::Union(const Rect& lhs, const Rect& rhs) {
	T left;
	T right;
	T bottom;
	T top;

	if constexpr (!InvertHorizontal) {
		left = std::min(lhs.left, rhs.left);
		right = std::max(lhs.right, rhs.right);
	}
	else {
		left = std::max(lhs.left, rhs.left);
		right = std::min(lhs.right, rhs.right);
	}

	if constexpr (!InvertVertical) {
		bottom = std::min(lhs.bottom, rhs.bottom);
		top = std::max(lhs.top, rhs.top);
	}
	else {
		bottom = std::max(lhs.bottom, rhs.bottom);
		top = std::min(lhs.top, rhs.top);
	}

	return Rect(left, right, bottom, top);
}

template <class T, bool InvertHorizontal, bool InvertVertical>
Rect<T, InvertHorizontal, InvertVertical> Rect<T, InvertHorizontal, InvertVertical>::Intersection(const Rect& lhs, const Rect& rhs) {
	T left;
	T right;
	T bottom;
	T top;

	if constexpr (!InvertHorizontal) {
		left = std::max(lhs.left, rhs.left);
		right = std::min(lhs.right, rhs.right);

		if (left > right)
			return Rect(0, 0, 0, 0);
	}
	else {
		left = std::min(lhs.left, rhs.left);
		right = std::max(lhs.right, rhs.right);

		if (right > left)
			return Rect(0, 0, 0, 0);
	}

	if constexpr (!InvertVertical) {
		bottom = std::max(lhs.bottom, rhs.bottom);
		top = std::min(lhs.top, rhs.top);

		if (bottom > top)
			return Rect(0, 0, 0, 0);
	}
	else {
		bottom = std::min(lhs.bottom, rhs.bottom);
		top = std::max(lhs.top, rhs.top);

		if (top > bottom)
			return Rect(0, 0, 0, 0);
	}

	return Rect(left, right, bottom, top);
}

template <class T, bool InvertHorizontal, bool InvertVertical>
bool Rect<T, InvertHorizontal, InvertVertical>::IsPointInside(const Vec2T& arg) const {
	auto minHoriz = std::min(left, right);
	auto maxHoriz = std::max(left, right);
	auto minVert = std::min(bottom, top);
	auto maxVert = std::max(bottom, top);

	return (minHoriz <= arg.x && arg.x <= maxHoriz) && (minVert <= arg.y && arg.y <= maxVert);
}

template <class T, bool InvertHorizontal, bool InvertVertical>
bool Rect<T, InvertHorizontal, InvertVertical>::IsRectInside(const Rect& arg) const {
	auto minHorizArg = std::min(arg.left, arg.right);
	auto maxHorizArg = std::max(arg.left, arg.right);
	auto minVertArg = std::min(arg.bottom, arg.top);
	auto maxVertArg = std::max(arg.bottom, arg.top);

	auto minHoriz = std::min(left, right);
	auto maxHoriz = std::max(left, right);
	auto minVert = std::min(bottom, top);
	auto maxVert = std::max(bottom, top);

	return (minHoriz <= minHorizArg && maxHorizArg <= maxHoriz) && (minVert <= minVertArg && maxVertArg <= maxVert);
}

template <class T, bool InvertHorizontal, bool InvertVertical>
bool Rect<T, InvertHorizontal, InvertVertical>::IsIntersecting(const Rect& arg) const {
	Rect intersection = Intersection(*this, arg);
	return intersection != Rect(0, 0, 0, 0);
}

template <class T, bool InvertHorizontal, bool InvertVertical>
bool Rect<T, InvertHorizontal, InvertVertical>::operator==(const Rect& rhs) const {
	return left == rhs.left && right == rhs.right && top == rhs.top && bottom == rhs.bottom;
}

template <class T, bool InvertHorizontal, bool InvertVertical>
bool Rect<T, InvertHorizontal, InvertVertical>::operator!=(const Rect& rhs) const {
	return !(*this == rhs);
}


using RectF = Rect<float, false, false>;
using RectI = Rect<int, false, false>;



} // namespace inl
