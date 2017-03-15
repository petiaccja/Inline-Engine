#pragma once
#include <BaseLibrary\Common_tmp.hpp>

class GuiControl
{
public:
	GuiControl(){}

	void SetBackgroundToColor(Color& color);
	void SetRect(int x, int y, int width, int height);

protected:
	Color color;
	Rect<int> rect;
};

inline void GuiControl::SetBackgroundToColor(Color& color)
{
	this->color = color;
}

inline void GuiControl::SetRect(int x, int y, int width, int height)
{
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;
}