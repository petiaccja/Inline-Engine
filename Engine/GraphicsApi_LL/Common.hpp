#pragma once

#include <cstdint>

struct Rectangle {
	Rectangle(int top, int bottom, int left, int right)
		:top(top), bottom(bottom), left(left), right(right) {}

	int Width() const { return right - left; }
	int Height() const { return top - bottom; }
	int CenterX() const { return (right + left) / 2; }
	int CenterY() const { return (top + bottom) / 2; }
	int Area() const { return Width() * Height(); }

	int top, bottom, left, right;
};


struct Cube {
	Cube(int top, int bottom, int left, int right, int front, int back)
		:top(top), bottom(bottom), left(left), right(right), front(front), back(back) {}

	int top, bottom, left, right, front, back;
};


struct ColorRGBA {
	ColorRGBA(float r = 0, float g = 0, float b = 0, float a = 1)
		: r(r), g(g), b(b), a(a) {}

	float r, g, b, a;
};


enum class eFormat {



};


struct TextureDescription {
	eFormat format;
	size_t width;
	size_t height;
	size_t depth;
};

enum class ePrimitiveTopology {
	POINTLIST = 1,
	LINELIST = 2,
	LINESTRIP = 3,
	TRIANGLELIST = 4,
	TRIANGLESTRIP = 5,
};

struct Viewport {
	float topLeftX;
	float topLeftY;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};