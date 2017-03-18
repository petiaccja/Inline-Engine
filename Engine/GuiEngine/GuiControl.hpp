#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include <unordered_map>

// TMP HEKK
#include <BaseLibrary\Platform\Window.hpp>
#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b

#include <gdiplus.h>
#undef min
#undef max

class GuiPlane;
class GuiText;
class GuiButton;

class GuiControl
{
public:
	GuiControl() {}

	template<class T>
	T* AddChild();

	GuiPlane*  AddChildPlane();
	GuiText*   AddChildText();
	GuiButton* AddChildButton();

	bool RemoveChild(GuiControl* child);

	virtual void OnPaint(HDC dc, Gdiplus::Graphics* graphics) {}

	void SetRect(int x, int y, int width, int height);

	const std::vector<GuiControl*>& GetChildren();

protected:
	Rect<int> rect;

	std::vector<GuiControl*> children;
	std::unordered_map<GuiControl*, size_t> childrenIndices; // For optimizing (AddChild & RemoveChild) functions
};

inline bool GuiControl::RemoveChild(GuiControl* child)
{
	auto it = childrenIndices.find(child);

	if (it != childrenIndices.end())
	{
		children.erase(children.begin() + it->second);
		childrenIndices.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

template<class T>
T* GuiControl::AddChild()
{
	T* child = new T();
	childrenIndices.insert(std::make_pair(child, children.size()));
	children.push_back(child);
	return child;
}

inline void GuiControl::SetRect(int x, int y, int width, int height)
{
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;

	for (GuiControl* child : children)
	{
		child->SetRect(x, y, width, height);
	}
}

inline const std::vector<GuiControl*>& GuiControl::GetChildren()
{
	return children;
}