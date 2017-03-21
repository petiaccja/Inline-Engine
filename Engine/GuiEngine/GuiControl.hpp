#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include <unordered_map>

// TMP HEKK (REMOVE)
#include <BaseLibrary\Platform\Window.hpp>
#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b
#include <gdiplus.h>
#undef min
#undef max
// TMP HEKK END

class CursorEvent
{
public:
	CursorEvent(ivec2 cursorPos): cursorPos(cursorPos){}

public:
	ivec2 cursorPos;
};

class GuiPlane;
class GuiText;
class GuiButton;

class GuiControl
{
public:
	GuiControl();

	virtual void OnPaint(HDC dc, Gdiplus::Graphics* graphics) {}

	template<class T>
	T* AddChild();

	GuiPlane*  AddPlane();
	GuiText*   AddText();
	GuiButton* AddButton();

	bool RemoveChild(GuiControl* child);

	void TraverseTowardParents(const std::function<void(GuiControl*)>& fn);

	void Move(float dx, float dy);
	void Move(const vec2& delta);

	void SetRect(float x, float y, float width, float height);
	void SetRect(Rect<float>& rect);

	const std::vector<GuiControl*>& GetChildren();
	Rect<float> GetRect();
	GuiControl* GetParent();

public:
	Delegate<void(CursorEvent&)> OnCursorClick;
	Delegate<void(CursorEvent&)> OnCursorPress;
	Delegate<void(CursorEvent&)> OnCursorRelease;
	Delegate<void(CursorEvent&)> OnCursorEnter;
	Delegate<void(CursorEvent&)> OnCursorLeave;
	Delegate<void(CursorEvent&)> OnCursorStay;
	Delegate<void(Rect<float>&)> OnTransformChanged;
	Delegate<void(GuiControl*)> OnParentChanged;

protected:
	Rect<float> rect;

	GuiControl* parent;
	GuiControl* front;
	GuiControl* back;

	std::vector<GuiControl*> children;
	std::unordered_map<GuiControl*, size_t> childrenIndices; // For optimizing (AddChild & RemoveChild) functions
};

inline GuiControl::GuiControl()
:parent(nullptr), front(nullptr), back(nullptr), rect(0, 0, 60, 20)
{

}

inline bool GuiControl::RemoveChild(GuiControl* child)
{
	auto it = childrenIndices.find(child);
	if (it != childrenIndices.end())
	{
		child->parent = nullptr;

		if (child->front)
			child->front->back = nullptr;

		if (child->back)
			child->back->front = nullptr;

		child->front = nullptr;
		child->back = nullptr;

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
	child->parent = this;
	child->OnParentChanged(this);

	if (children.size() != 0)
	{
		GuiControl* lastControl = children[children.size() - 1];
		child->back = lastControl;
		lastControl->front = child;
	}

	childrenIndices.insert(std::make_pair(child, children.size()));
	children.push_back(child);
	return child;
}

inline void GuiControl::TraverseTowardParents(const std::function<void(GuiControl*)>& fn)
{
	fn(this);

	if (back)
		back->TraverseTowardParents(fn);
	else if (parent)
		fn(parent);
}

inline void GuiControl::SetRect(float x, float y, float width, float height)
{
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;

	OnTransformChanged(rect);
}

inline void GuiControl::SetRect(Rect<float>& rect)
{
	SetRect(rect.x, rect.y, rect.width, rect.height);
}


inline void GuiControl::Move(float dx, float dy)
{
	rect.x += dx;
	rect.y += dy;

	for (GuiControl* child : children)
		child->Move(dx, dy);
}

inline void GuiControl::Move(const vec2& delta)
{
	Move(delta.x, delta.y);
}

inline const std::vector<GuiControl*>& GuiControl::GetChildren()
{
	return children;
}

inline Rect<float> GuiControl::GetRect()
{
	return rect;
}

inline GuiControl* GuiControl::GetParent()
{
	return parent;
}