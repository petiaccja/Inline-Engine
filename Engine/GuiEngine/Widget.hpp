#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiEvent.h"

// TMP HEKK (REMOVE)
#include <BaseLibrary\Platform\Window.hpp>
#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b
#include <gdiplus.h>
#undef min
#undef max
// TMP HEKK END

#include <unordered_map>

class GuiEngine;
class Widget;
class GuiText;
class GuiButton;
class GuiList;
class GuiSlider;

class Widget
{
public:
	Widget();
	Widget(GuiEngine* guiEngine);
	Widget(GuiEngine* guiEngine, bool bLayer);

	virtual ~Widget() { Clear(); }
	void Clear();

	// Important to implement in derived classes
	virtual Widget* Clone() const { return new Widget(*this); }
	Widget& operator = (const Widget& other);

	template<class T>
	T* Add();

	void		Add(Widget* child);
	Widget*		AddWidget();
	GuiText*	AddText();
	GuiButton*	AddButton();
	GuiList*	AddList();

	bool RemoveChild(Widget* child);
	bool Remove();

	void TraverseTowardParents(const std::function<void(Widget*)>& fn);

	void Move(float dx, float dy);
	void Move(const vec2& delta) { Move(delta.x, delta.y); }

	void FillParent() { SetRect(parent->GetRect()); }
	void Stretch() {}

	Widget*  AsPlane()  { return (Widget*)this; }
	GuiText*   AsText()   { return (GuiText*)this; }
	GuiButton* AsButton() { return (GuiButton*)this; }
	GuiList*   AsList()   { return (GuiList*)this; }
	GuiSlider* AsSlider() { return (GuiSlider*)this; }

	void SetRect(float x, float y, float width, float height);
	void SetRect(const Rect<float>& rect)	{ SetRect(rect.x, rect.y, rect.width, rect.height); }
	void SetName(const std::wstring& str)	{ name = str; }
	void SetName(const std::string& str)	{ SetName(std::wstring(str.begin(), str.end())); }
	void SetContextMenu(Widget* c)		{ contextMenu = c; }
	void SetPos(const vec2& p)				{ SetPos(p.x, p.y); }
	void SetPos(float x, float y)			{ SetRect(x, y, size.x, size.y); }
	void SetCenterPos(float x, float y)		{ SetPos(x - GetHalfWidth(), y + GetHalfHeight()); }
	void SetPosX(float x)					{ SetRect(x, pos.y, size.x, size.y); }
	void SetPosY(float y)					{ SetRect(pos.x, y, size.x, size.y); }
	void SetSize(const vec2& s)				{ SetSize(s.x, s.y); }
	void SetSize(float width, float height) { SetRect(pos.x, pos.y, width, height); }
	void SetWidth(float w)					{ SetSize(vec2(w, size.y)); }
	void SetHeight(float h)					{ SetSize(vec2(size.x, h)); }

	void SetClientSize(const vec2& s) { SetClientSize(s.x, s.y); }
	void SetClientSize(float width, float height) { SetSize(width + borderPixelSize * 2, height + borderPixelSize * 2); }

	void SetEventPropagationPolicy(eEventPropagationPolicy e) { eventPropagationPolicy = e; }

	void EnableClip() { SetClip(true); }
	void DisableClip() { SetClip(false); }
	void SetClip(bool b) { bClip = b; }

	void SetBgToColor(const Color& idleColor, const Color& hoverColor);
	void SetBgIdleColor(const Color& color);
	void SetBgHoverColor(const Color& color);
	void SetBgActiveColor(const Color& color) { bgActiveColor = color; }
	void SetBgActiveColorToIdle();
	void SetBgActiveColorToHover();
	void SetBgColorForAllStates(const Color& color);

	void SetBgIdleImage(const std::wstring& filePath);
	void SetBgHoverImage(const std::wstring& filePath);
	void SetBgActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }
	void SetBgActiveImageToIdle();
	void SetBgActiveImageToHover();
	void SetBgImageForAllStates(const std::wstring& filePath);

	void HideBgImage() { SetBgImageVisibility(false); }
	void HideBgColor() { SetBgColorVisibility(false); }

	void ShowBgImage() { SetBgImageVisibility(true); }
	void ShowBgColor() { SetBgColorVisibility(true); }

	void SetBgImageVisibility(bool bVisible) { bBgImageVisible = bVisible; }
	void SetBgColorVisibility(bool bVisible) { bBgColorVisible = bVisible; }

	float GetClientSpaceCursorPosX();
	float GetClientSpaceCursorPosY();

	float GetPosX()		  { return pos.x; }
	float GetPosY()		  { return pos.y; }
	const vec2& GetPos()  { return pos; }
	float GetCenterPosX() { return pos.x + GetHalfWidth(); }
	float GetCenterPosY() { return pos.y + GetHalfHeight(); }
	vec2 GetCenterPos()  { return pos + GetHalfSize(); }
	const vec2& GetSize() { return size; }
	float GetWidth()	  { return size.x; }
	float GetHeight()	  { return size.y; }
	float GetHalfWidth()  { return GetWidth() * 0.5f; }
	float GetHalfHeight() { return GetHeight() * 0.5f; }
	vec2 GetHalfSize() { return vec2(GetHalfWidth(), GetHalfHeight()); }

	float GetClientPosX() { return pos.x + borderPixelSize; }
	float GetClientPosY() { return pos.y + borderPixelSize; }
	vec2 GetClientPos() { return pos + vec2(borderPixelSize, borderPixelSize); }
	float GetClientCenterPosY() { return GetClientPosY() - GetHalfHeight(); }
	float GetClientCenterPosX() { return GetClientPosX() + GetHalfWidth(); }
	vec2 GetClientCenterPos() { return GetClientPos() + GetClientHalfSize(); }
	vec2 GetClientSize() { return size - vec2(borderPixelSize, borderPixelSize) * 2; }
	float GetClientWidth() { return size.x - borderPixelSize * 2; }
	float GetClientHeight() { return size.y - borderPixelSize * 2; }
	float GetClientHalfWidth() { return GetClientWidth() * 0.5f; }
	float GetClientHalfHeight() { return GetClientWidth() * 0.5f; }
	vec2 GetClientHalfSize() { return vec2(GetClientWidth() * 0.5f, GetClientHeight() * 0.5f); }

	Rect<float> GetRect() { return Rect<float>(pos.x, pos.y, size.x, size.y); }
	Rect<float> GetClientRect() { return Rect<float>(pos.x + borderPixelSize, pos.y + borderPixelSize, size.x - borderPixelSize * 2, size.y - borderPixelSize * 2); }

	Widget* GetParent() { return parent; }
	Widget* GetContextMenu() { return contextMenu; }
	const std::vector<Widget*>& GetChildren() { return children; }

	template<class T>
	T* GetChildByIdx(int index) { return (T*)GetChildren()[index]; }

	Widget* GetChildByIdx(int index) { return GetChildren()[index]; }
	int GetIdx() { return idx; }

	eEventPropagationPolicy GetEventPropagationPolicy() { return eventPropagationPolicy; }

	Color& GetBgActiveColor() { return bgActiveColor; }
	Color& GetBgIdleColor() { return bgIdleColor; }
	Color& GetBgHoverColor() { return bgHoverColor; }

	Gdiplus::Bitmap* GetBgActiveImage() { return bgActiveImage; }
	Gdiplus::Bitmap* GetBgIdleImage() { return bgIdleImage; }
	Gdiplus::Bitmap* GetBgHoverImage() { return bgHoverImage; }


	bool IsPointInside(ivec2 pt) { return GetRect().IsPointInside(pt); }
	bool IsLayer() { return bLayer; }

	void SetBorder(float borderPixelSize, const Color& borderColor) { this->borderPixelSize = borderPixelSize; this->borderColor = borderColor; }
	const Color& GetBorderColor() { return borderColor; }
	float GetBorderPixelSize() { return borderPixelSize; }

protected:
	void SetActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }

protected:
	// Name it however you want
	std::wstring name;

	// Position
	vec2 pos;

	// Width, Height
	vec2 size;

	// Parent widget
	Widget* parent;

	// IsLayer ?
	bool bLayer;

	// Is clipped by parent client area?
	bool bClip;

	// children index in parent
	int idx;

	// Children widgets
	std::vector<Widget*> children;
	std::unordered_map<Widget*, size_t> childrenIndices; // For optimizing (Add & Remove) functions

	// What this widget should do with the message it reaches while moving up in hierarchy (toward parents)
	eEventPropagationPolicy eventPropagationPolicy;

	// Layering (Our neighbours in the tree hierarchy)
	Widget* front;
	Widget* back;

	// Attached context menu
	Widget* contextMenu;

	// Background image and color
	bool bBgImageVisible;
	bool bBgColorVisible;
	Gdiplus::Bitmap* bgIdleImage;
	Gdiplus::Bitmap* bgHoverImage;
	Gdiplus::Bitmap* bgActiveImage;
	Color bgIdleColor;
	Color bgHoverColor;
	Color bgActiveColor;

	// Border
	Color borderColor;
	float borderPixelSize;

	// Access to god
	GuiEngine* guiEngine;

public:
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseClick;
	Delegate<void(Widget* self, CursorEvent& evt)> onMousePress;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseRelease;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseMove;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseEnter;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseLeave;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseHover;

	Delegate<void(Widget* self, Rect<float>& rect)> onTransformChange;
	Delegate<void(Widget* self, Rect<float>& rect)> onParentTransformChange;
	Delegate<void(Widget* self, Widget* parent)> onParentChange;

	Delegate<void(Widget* self, Gdiplus::Graphics* graphics, Rect<float>& clipRect)> onPaint;
	Delegate<void(Widget* self, float deltaTime)> onUpdate;
};

inline Widget::Widget(GuiEngine* guiEngine, bool bLayer)
:guiEngine(guiEngine), pos(0, 0), eventPropagationPolicy(eEventPropagationPolicy::PROCESS), size(60, 20), idx(-1), bLayer(bLayer), parent(nullptr), front(nullptr), back(nullptr), contextMenu(nullptr), bgIdleColor(45), bgHoverColor(75), bgActiveImage(nullptr), bgIdleImage(nullptr), bgHoverImage(nullptr), borderPixelSize(0), borderColor(128), bBgImageVisible(true), bBgColorVisible(true)
{
	SetBgActiveColor(bgIdleColor);

	onMouseEnter += [](Widget* self, CursorEvent& event)
	{
		self->SetBgActiveColor(self->GetBgHoverColor());
		self->SetBgActiveImage(self->GetBgHoverImage());
	};

	onMouseLeave += [](Widget* self, CursorEvent& event)
	{
		self->SetBgActiveColor(self->GetBgIdleColor());
		self->SetBgActiveImage(self->GetBgIdleImage());
	};

	onPaint += [](Widget* self, Gdiplus::Graphics* graphics, Rect<float>& clipRect)
	{
		Rect<float> clientRect = self->GetClientRect();
		Rect<float> rect = self->GetRect();

		Gdiplus::Rect gdiBorderRect(rect.x, rect.y, rect.width, rect.height);
		Gdiplus::Rect gdiClientRect(clientRect.x, clientRect.y, clientRect.width, clientRect.height);
		Gdiplus::Rect gdiClipRect(clipRect.x, clipRect.y, clipRect.width, clipRect.height);

		//Before we draw everything use the clip rectangle
		if(self->bClip)
			graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeReplace);

		// Draw border rectangle
		if (self->borderPixelSize != 0)
		{
			Gdiplus::SolidBrush brush(Gdiplus::Color(self->borderColor.a, self->borderColor.r, self->borderColor.g, self->borderColor.b));
			graphics->FillRectangle(&brush, gdiBorderRect);
		}

		if (self->GetBgActiveImage() && self->bBgImageVisible)
		{
			graphics->DrawImage(self->GetBgActiveImage(), gdiClientRect);
		}
		else if(self->bBgColorVisible)
		{
			// Draw main rectangle
			Color bgColor = self->GetBgActiveColor();
			Gdiplus::SolidBrush brush(Gdiplus::Color(bgColor.a, bgColor.r, bgColor.g, bgColor.b));
			graphics->FillRectangle(&brush, gdiClientRect);
		}
	};
}

inline Widget::Widget(GuiEngine* guiEngine)
:Widget(guiEngine, false)
{

}

inline Widget::Widget()
:Widget(nullptr, false)
{

}

inline void Widget::Clear()
{
	Remove();

	front = nullptr;
	back = nullptr;
	parent = nullptr;
	idx = -1;

	for (Widget* c : children)
		delete c;

	children.clear();
	childrenIndices.clear();
}

inline Widget& Widget::operator = (const Widget& other)
{
	Clear();

	guiEngine = other.guiEngine;
	pos = other.pos;
	size = other.size;
	name = other.name;
	onMouseClick = other.onMouseClick;
	onMousePress = other.onMousePress;
	onMouseRelease = other.onMouseRelease;
	onMouseMove = other.onMouseMove;
	onMouseEnter = other.onMouseEnter;
	onMouseLeave = other.onMouseLeave;
	onMouseHover = other.onMouseHover;
	onTransformChange = other.onTransformChange;
	onParentTransformChange = other.onParentTransformChange;
	onPaint = other.onPaint;
	onUpdate = other.onUpdate;
	idx = other.idx;
	bLayer = other.bLayer;
	borderColor = other.borderColor;
	borderPixelSize = other.borderPixelSize;
	
	// Background
	if (other.bgIdleImage)
		bgIdleImage = other.bgIdleImage->Clone(Gdiplus::RectF(0, 0, other.bgIdleImage->GetWidth(), other.bgIdleImage->GetHeight()), other.bgIdleImage->GetPixelFormat());

	if (other.bgHoverImage)
		bgHoverImage = other.bgHoverImage->Clone(Gdiplus::RectF(0, 0, other.bgHoverImage->GetWidth(), other.bgHoverImage->GetHeight()), other.bgHoverImage->GetPixelFormat());

	if (other.bgActiveImage == other.bgIdleImage)
		bgActiveImage = bgIdleImage;
	else if (other.bgActiveImage == other.bgHoverImage)
		bgActiveImage = bgHoverImage;

	bgActiveColor = other.bgActiveColor;
	bgIdleColor = other.bgIdleColor;
	bgHoverColor = other.bgHoverColor;

	// Context menu
	if(other.contextMenu)
		contextMenu = other.contextMenu->Clone();

	for (Widget* child : other.children)
		Add(child->Clone());

	// We are root, so attach to other's parent..
	if (other.parent && other.parent->IsLayer())
		other.parent->Add(this);

	return *this;
}

template<class T>
T* Widget::Add()
{
	T* child = new T(guiEngine);
	Add(child);
	return child;
}

inline void Widget::Add(Widget* child)
{
	if (child->parent)
		child->parent->RemoveChild(child);

	child->parent = this;

	if (children.size() != 0)
	{
		Widget* lastControl = children[children.size() - 1];
		child->back = lastControl;
		lastControl->front = child;
	}

	childrenIndices.insert(std::make_pair(child, children.size()));
	child->idx = children.size();
	children.push_back(child);

	child->onParentChange(child, this);
}

inline bool Widget::RemoveChild(Widget* child)
{
	auto it = childrenIndices.find(child);
	if (it != childrenIndices.end())
	{
		if (child->front)
			child->front->back = nullptr;

		if (child->back)
			child->back->front = nullptr;

		child->parent = nullptr;
		child->idx = -1;
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

inline bool Widget::Remove()
{
	if (parent)
		return parent->RemoveChild(this);

	return false;
}

inline void Widget::TraverseTowardParents(const std::function<void(Widget*)>& fn)
{
	// Terminate recursive call
	if (eventPropagationPolicy == eEventPropagationPolicy::STOP)
		return;

	// Process fn for this control
	if (eventPropagationPolicy == eEventPropagationPolicy::PROCESS || eventPropagationPolicy == eEventPropagationPolicy::PROCESS_STOP)
		fn(this);

	// Continue recursive calls
	if (eventPropagationPolicy == eEventPropagationPolicy::PROCESS || eventPropagationPolicy == eEventPropagationPolicy::AVOID)
	{
		if (back)
			back->TraverseTowardParents(fn);
		else if (parent)
			fn(parent);// parent->TraverseTowardParents(fn);
	}
}

inline void Widget::Move(float dx, float dy)
{
	pos.x += dx;
	pos.y += dy;

	for (Widget* child : children)
		child->Move(dx, dy);
}

inline void Widget::SetRect(float x, float y, float width, float height)
{
	Rect<float> oldRect = GetRect();
	pos.x = x;
	pos.y = y;
	size.x = width;
	size.y = height;
	Rect<float> rect = GetRect();

	if (rect != oldRect)
	{
		onTransformChange(this, rect);

		for (Widget* child : children)
		{
			//child->Move(rect.x - oldRect.x, rect.y - oldRect.y);
			child->onParentTransformChange(child, rect);
		}
	}
}

inline void Widget::SetBgIdleImage(const std::wstring& str)
{
	Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(str.c_str());

	if (bgIdleImage == GetBgActiveImage())
		SetBgActiveImage(newBitmap);

	if (bgIdleImage)
		delete bgIdleImage;

	bgIdleImage = newBitmap;
}

inline void Widget::SetBgHoverImage(const std::wstring& str)
{
	Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(str.c_str());

	if (bgHoverImage == GetBgActiveImage())
		SetBgActiveImage(newBitmap);

	if (bgHoverImage)
		delete bgHoverImage;

	bgHoverImage = newBitmap;
}

inline void Widget::SetBgIdleColor(const Color& color)
{
	if (bgIdleColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgIdleColor = color;
}

inline void Widget::SetBgToColor(const Color& idleColor, const Color& hoverColor)
{
	// Disable Image !
	HideBgImage();

	SetBgHoverColor(hoverColor);
	SetBgIdleColor(idleColor);
}

inline void Widget::SetBgHoverColor(const Color& color)
{
	if (bgHoverColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgHoverColor = color;
}

inline void Widget::SetBgActiveColorToIdle()
{
	SetBgActiveColor(GetBgIdleColor());
}

inline void Widget::SetBgActiveColorToHover()
{
	SetBgActiveColor(GetBgHoverColor());
}

inline void Widget::SetBgColorForAllStates(const Color& color)
{
	SetBgHoverColor(color);
	SetBgIdleColor(color);
}

inline void Widget::SetBgActiveImageToIdle()
{
	SetBgActiveImage(bgIdleImage);
}

inline void Widget::SetBgActiveImageToHover()
{
	SetBgActiveImage(bgHoverImage);
}

inline void Widget::SetBgImageForAllStates(const std::wstring& filePath)
{
	SetBgIdleImage(filePath);
	SetBgHoverImage(filePath);
}