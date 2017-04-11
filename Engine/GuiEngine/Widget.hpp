#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiEvent.h"

// TMP HEKK (REMOVE)
#include <BaseLibrary\Platform\Window.hpp>
//#define min(a,b) a < b ? a : b
//#define max(a,b) a > b ? a : b
//#include <gdiplus.h>
//#undef min
//#undef max
//// TMP HEKK END

#include <unordered_map>

class GuiEngine;
class Widget;
class GuiText;
class GuiButton;
class GuiList;
class GuiSlider;
class GuiCollapsable;

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

	bool Remove(Widget* child);
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
	GuiCollapsable* AsCollapsable() { return (GuiCollapsable*)this; }


	void SetClientRect(float x, float y, float width, float height);
	void SetRect(float x, float y, float width, float height);
	void SetClientSize(float width, float height) { SetClientRect(GetClientRect().left, GetClientRect().top, width, height); }

	void SetClientRect(const RectF rect) { SetClientRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()); }
	void SetRect(const RectF& rect)	{ SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()); }

	void SetName(const std::wstring& str)	{ name = str; }
	void SetName(const std::string& str)	{ SetName(std::wstring(str.begin(), str.end())); }
	void SetContextMenu(Widget* c)			{ contextMenu = c; }
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

	void SetEventPropagationPolicy(eEventPropagationPolicy e) { eventPropagationPolicy = e; }

	void EnableClipChildren() { SetClipChildren(true); }
	void DisableClipChildren() { SetClipChildren(false); }
	void SetClipChildren(bool b) { bClipChildren = b; }

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

	void SetBorder(float leftLength, float rightLength, float topLength, float bottomLength, const Color& color);
	void SetBorder(float borderLength, const Color& color) { SetBorder(borderLength, borderLength, borderLength, borderLength, color); }
	
	void SetMargin(float leftLength, float topLength, float rightLength, float bottomLength);
	void SetPadding(float leftLength, float topLength, float rightLength, float bottomLength);
	void SetMargin(float length) { SetMargin(length, length, length, length); }
	void SetPadding(float length) { SetPadding(length, length, length, length); }

	void SetBgImageVisibility(bool bVisible) { bBgImageVisible = bVisible; }
	void SetBgColorVisibility(bool bVisible) { bBgColorVisible = bVisible; }

	void SetFitToChildren(bool bFit) { bFitToChildren = bFit; }

	void HideBgImage() { SetBgImageVisibility(false); }
	void HideBgColor() { SetBgColorVisibility(false); }

	void ShowBgImage() { SetBgImageVisibility(true); }
	void ShowBgColor() { SetBgColorVisibility(true); }

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

	const RectF& GetPadding() const { return padding; }
	const RectF& GetMargin() const { return margin; }

	float GetClientPosX() { return GetClientRect().left; }
	float GetClientPosY() { return GetClientRect().top; }
	vec2 GetClientPos() { return GetClientRect().GetPos(); }
	float GetClientCenterPosY() { return GetClientPosY() - GetClientHalfHeight(); }
	float GetClientCenterPosX() { return GetClientPosX() + GetClientHalfWidth(); }
	vec2 GetClientCenterPos() { return GetClientPos() + GetClientHalfSize(); }
	vec2 GetClientSize() { return GetClientRect().GetSize(); }
	float GetClientWidth() { return GetClientRect().GetWidth(); }
	float GetClientHeight() { return GetClientRect().GetHeight(); }
	float GetClientHalfWidth() { return GetClientWidth() * 0.5f; }
	float GetClientHalfHeight() { return GetClientWidth() * 0.5f; }
	vec2 GetClientHalfSize() { return GetClientSize() * 0.5f; }

	RectF GetRect();
	RectF GetClientRect();
	RectF GetPaddingRect();
	RectF GetBorderRect();

	Widget* GetParent() { return parent; }
	Widget* GetContextMenu() { return contextMenu; }
	std::vector<Widget*>& GetChildren() { return children; }
	RectF GetChildrenBoundRect();

	template<class T>
	T* GetChildByIdx(int index) { return (T*)GetChildren()[index]; }

	Widget* GetChildByIdx(int index) { return GetChildren()[index]; }
	int GetIndexInParent() { return indexInParent; }

	eEventPropagationPolicy GetEventPropagationPolicy() { return eventPropagationPolicy; }

	Color& GetBgActiveColor() { return bgActiveColor; }
	Color& GetBgIdleColor() { return bgIdleColor; }
	Color& GetBgHoverColor() { return bgHoverColor; }

	Gdiplus::Bitmap* GetBgActiveImage() { return bgActiveImage; }
	Gdiplus::Bitmap* GetBgIdleImage() { return bgIdleImage; }
	Gdiplus::Bitmap* GetBgHoverImage() { return bgHoverImage; }


	bool IsPointInside(ivec2 pt) { return GetRect().IsPointInside(pt); }

	bool IsLayer() { return bLayer; }
	bool IsChildrenClipEnabled() { return bClipChildren; }

	const Color& GetBorderColor() { return borderColor; }
	RectF GetBorder() { return border; }

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
	bool bClipChildren;

	// If true parent rectangle will be exactly aroound childs always
	bool bFitToChildren;

	// children index in parent
	int indexInParent;
	
	// Children widgets
	std::vector<Widget*> children;

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
	RectF border;

	// Margin
	RectF margin;

	// Padding
	RectF padding;

	// Access to god
	GuiEngine* guiEngine;

public:
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseClicked;
	Delegate<void(Widget* self, CursorEvent& evt)> onMousePressed;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseReleased;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseMoved;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseEntered;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseLeaved;
	Delegate<void(Widget* self, CursorEvent& evt)> onMouseHovered;

	Delegate<void(Widget* self, RectF& rect)> onTransformChanged;
	Delegate<void(Widget* self, RectF& rect)> onParentTransformChanged;
	Delegate<void(Widget* self, RectF& rect)> onChildTransformChanged;
	Delegate<void(Widget* self, Widget* parent)> onParentChanged;
	Delegate<void(Widget* self, Widget* child)> onChildAdded;
	Delegate<void(Widget* self, Widget* child)> onChildRemoved;

	Delegate<void(Widget* self, Gdiplus::Graphics* graphics, RectF& clipRect)> onPaint;
	Delegate<void(Widget* self, float deltaTime)> onUpdate;

	//virtual void OnMouseClicked(CursorEvent& evt) {}
	//virtual void OnMousePressed(CursorEvent& evt) {}
	//virtual void OnMouseReleased(CursorEvent& evt) {}
	//virtual void OnMouseMoved(CursorEvent& evt) {}
	//virtual void OnMouseEntered(CursorEvent& evt) {}
	//virtual void OnMouseLeaved(CursorEvent& evt) {}
	//virtual void OnMouseHovered(CursorEvent& evt) {}
	//virtual void OnTransformChanged(Widget* self, RectF& rect) {}
	//virtual void OnParentTransformChanged(Widget* self, RectF& rect) {}
	//virtual void OnChildTransformChanged(Widget* self, RectF& rect) {}
	//virtual void OnParentChanged(Widget* self, Widget* parent) {}
	//virtual void OnChildAdded(Widget* self, Widget* child) {}
	//virtual void OnChildRemoved(Widget* self, Widget* child) {}
	//
	//virtual void OnPaint(Widget* self, Gdiplus::Graphics* graphics, RectF& clipRect) {}
	//virtual void OnUpdate(Widget* self, float deltaTime) {}
};

inline Widget::Widget(GuiEngine* guiEngine, bool bLayer)
:guiEngine(guiEngine), pos(0, 0), eventPropagationPolicy(eEventPropagationPolicy::PROCESS), size(60, 20), indexInParent(-1), bLayer(bLayer), parent(nullptr), front(nullptr), back(nullptr), contextMenu(nullptr), bgIdleColor(45), bgHoverColor(75), bgActiveImage(nullptr), bgIdleImage(nullptr), bgHoverImage(nullptr), border(0,0,0,0), borderColor(128), bBgImageVisible(true), bBgColorVisible(true), bClipChildren(true), bFitToChildren(false), margin(0,0,0,0), padding(0,0,0,0)
{
	SetBgActiveColor(bgIdleColor);

	onMouseEntered += [](Widget* self, CursorEvent& event)
	{
		self->SetBgActiveColor(self->GetBgHoverColor());
		self->SetBgActiveImage(self->GetBgHoverImage());
	};

	onMouseLeaved += [](Widget* self, CursorEvent& event)
	{
		self->SetBgActiveColor(self->GetBgIdleColor());
		self->SetBgActiveImage(self->GetBgIdleImage());
	};

	onPaint += [](Widget* self, Gdiplus::Graphics* graphics, RectF& clipRect)
	{
		RectF paddingRect = self->GetPaddingRect();
		RectF borderRect = self->GetBorderRect();
		const RectF& border = self->border;

		Gdiplus::Rect gdiBorderRect(borderRect.left, borderRect.top, borderRect.GetWidth(), borderRect.GetHeight());
		Gdiplus::Rect gdiPaddingRect(paddingRect.left, paddingRect.top, paddingRect.GetWidth(), paddingRect.GetHeight());
		Gdiplus::Rect gdiClipRect(clipRect.left, clipRect.top, clipRect.GetWidth(), clipRect.GetHeight());

		// Clipping
		graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeReplace);

		// Draw left border
		Gdiplus::SolidBrush borderBrush(Gdiplus::Color(self->borderColor.a, self->borderColor.r, self->borderColor.g, self->borderColor.b));
		if (border.left != 0)
		{
			RectF tmp = borderRect;

			// Setup left border
			tmp.right = tmp.left + border.left;

			Gdiplus::Rect tmpGdi(tmp.left, tmp.top, tmp.GetWidth(), tmp.GetHeight());
			graphics->FillRectangle(&borderBrush, tmpGdi);
		}

		// Draw right border
		if (border.right != 0)
		{
			RectF tmp = borderRect;

			// Setup right border
			tmp.left = tmp.right - border.right;

			Gdiplus::Rect tmpGdi(tmp.left, tmp.top, tmp.GetWidth(), tmp.GetHeight());
			graphics->FillRectangle(&borderBrush, tmpGdi);
		}
		
		// Draw top border
		if (border.top != 0)
		{
			RectF tmp = borderRect;

			// Setup top border
			tmp.bottom = tmp.top + border.top;

			Gdiplus::Rect tmpGdi(tmp.left, tmp.top, tmp.GetWidth(), tmp.GetHeight());
			graphics->FillRectangle(&borderBrush, tmpGdi);
		}

		// Draw bottom border
		if (border.bottom != 0)
		{
			RectF tmp = borderRect;

			// Setup top border
			tmp.top = tmp.bottom - border.bottom;

			Gdiplus::Rect tmpGdi(tmp.left, tmp.top, tmp.GetWidth(), tmp.GetHeight());
			graphics->FillRectangle(&borderBrush, tmpGdi);
		}


		// Draw Background Image Rectangle
		if (self->GetBgActiveImage() && self->bBgImageVisible)
		{
			graphics->DrawImage(self->GetBgActiveImage(), gdiPaddingRect);
		}
		else if(self->bBgColorVisible) // Draw Background Colored Rectangle
		{
			Color bgColor = self->GetBgActiveColor();
			Gdiplus::SolidBrush brush(Gdiplus::Color(bgColor.a, bgColor.r, bgColor.g, bgColor.b));
			graphics->FillRectangle(&brush, gdiPaddingRect);
		}
	};

	// TODO REMOVE THESE
	onChildAdded += [](Widget* self, Widget* child)
	{
		if (self->bFitToChildren)
		{
			RectF boundRect = self->GetChildrenBoundRect();
			self->SetClientSize(boundRect.GetSize());
		}
	};
	
	onChildRemoved += [](Widget* self, Widget* child)
	{
		if (self->bFitToChildren)
		{
			RectF boundRect = self->GetChildrenBoundRect();
			self->SetClientSize(boundRect.GetSize());
		}
	};
	
	onChildTransformChanged += [](Widget* self, RectF& rect)
	{
		if (self->bFitToChildren)
		{
			RectF boundRect = self->GetChildrenBoundRect();
			self->SetClientSize(boundRect.GetSize());
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
	indexInParent = -1;

	for (Widget* c : children)
		delete c;

	children.clear();
}

inline Widget& Widget::operator = (const Widget& other)
{
	Clear();

	guiEngine = other.guiEngine;
	pos = other.pos;
	size = other.size;
	name = other.name;
	onMouseClicked = other.onMouseClicked;
	onMousePressed = other.onMousePressed;
	onMouseReleased = other.onMouseReleased;
	onMouseMoved = other.onMouseMoved;
	onMouseEntered = other.onMouseEntered;
	onMouseLeaved = other.onMouseLeaved;
	onMouseHovered = other.onMouseHovered;
	onTransformChanged = other.onTransformChanged;
	onParentTransformChanged = other.onParentTransformChanged;
	onPaint = other.onPaint;
	onUpdate = other.onUpdate;
	indexInParent = other.indexInParent;
	bLayer = other.bLayer;
	bClipChildren = other.bClipChildren;
	bFitToChildren = other.bFitToChildren;
	indexInParent = other.indexInParent;
	borderColor = other.borderColor;
	border = other.border;
	margin = other.margin;
	padding = other.padding;

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
		child->parent->Remove(child);

	child->parent = this;

	if (children.size() != 0)
	{
		Widget* lastControl = children[children.size() - 1];
		child->back = lastControl;
		lastControl->front = child;
	}

	child->indexInParent = children.size();
	children.push_back(child);

	child->onParentChanged(child, this);
	onChildAdded(this, child);
}

inline bool Widget::Remove(Widget* child)
{
	int potentialIndex = child->indexInParent;
	std::vector<Widget*>& children = GetChildren();

	if (potentialIndex < children.size())
	{
		Widget* potentialMatch = children[potentialIndex];

		if (child == potentialMatch)
		{
			if (child->front)
				child->front->back = nullptr;

			if (child->back)
				child->back->front = nullptr;

			child->parent = nullptr;
			child->indexInParent = -1;
			child->front = nullptr;
			child->back = nullptr;

			// Erase
			children.erase(children.begin() + potentialIndex);

			// After erasing it's important to correct the neighbours widget's indexes
			for (int i = potentialIndex; i < children.size(); ++i)
				if (children[i]->indexInParent != -1)
					--children[i]->indexInParent;

			onChildRemoved(this, child);
			child->onParentChanged(child, nullptr);

			return true;
		}
	}
	return false;
}

inline bool Widget::Remove()
{
	if (parent)
		return parent->Remove(this);

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
	SetPos(pos.x + dx, pos.y + dy);
}

inline void Widget::SetRect(float x, float y, float width, float height)
{
	RectF oldRect = GetRect();

	pos.x = x;
	pos.y = y;
	size.x = width;
	size.y = height;
	RectF rect = GetRect();

	if (rect != oldRect)
	{
		for (Widget* child : children)
		{
			child->Move(rect.GetPos() - oldRect.GetPos());
			child->onParentTransformChanged(child, rect);
		}
		
		onTransformChanged(this, rect);

		if (parent)
			parent->onChildTransformChanged(parent, rect);
	}
}

inline void Widget::SetClientRect(float x, float y, float width, float height)
{
	RectF resultRect = RectF(x, y, x + width, y + height);

	// Convert length to signed offset
	RectF padding_ = padding;
	RectF border_ = border;
	RectF margin_ = margin;
	padding_.left *= -1;
	padding_.top *= -1;
	border_.left *= -1;
	border_.top *= -1;
	margin_.left *= -1;
	margin_.top *= -1;

	// Now we can move the sides in appropriate directions
	resultRect.MoveSides(padding_);
	resultRect.MoveSides(border_);
	resultRect.MoveSides(margin_);

	SetRect(resultRect);
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
	ShowBgColor();
}

inline void Widget::SetBgToColor(const Color& idleColor, const Color& hoverColor)
{
	// Disable Image !
	HideBgImage();

	SetBgHoverColor(hoverColor);
	SetBgIdleColor(idleColor);
	ShowBgColor();
}

inline void Widget::SetBgHoverColor(const Color& color)
{
	if (bgHoverColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgHoverColor = color;
	ShowBgColor();
}

inline void Widget::SetBgActiveColorToIdle()
{
	SetBgActiveColor(GetBgIdleColor());
	ShowBgColor();
}

inline void Widget::SetBgActiveColorToHover()
{
	SetBgActiveColor(GetBgHoverColor());
	ShowBgColor();
}

inline void Widget::SetBgColorForAllStates(const Color& color)
{
	SetBgHoverColor(color);
	SetBgIdleColor(color);
	ShowBgColor();
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

inline RectF Widget::GetChildrenBoundRect()
{
	// NOOB SOLUTION QUERY CHILDS
	auto& children = GetChildren();

	RectF idealRect;

	if (children.size() != 0)
		idealRect = children[0]->GetRect();

	for (int i = 1; i < children.size(); ++i)
		idealRect = children[i]->GetRect().Union(idealRect);

	return idealRect;
}

inline RectF Widget::GetRect()
{
	return RectF::FromSize(pos.x, pos.y, size.x, size.y);
}

inline RectF Widget::GetClientRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-margin);
	result.MoveSidesLocal(-border);
	result.MoveSidesLocal(-padding);

	return result;
}

inline RectF Widget::GetPaddingRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-margin);
	result.MoveSidesLocal(-border);

	return result;
}

inline RectF Widget::GetBorderRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-margin);

	return result;
}

inline void Widget::SetMargin(float leftLength, float topLength, float rightLength, float bottomLength)
{
	margin = RectF(leftLength, topLength, rightLength, bottomLength);

	//RectF newMargin = RectF(leftLength, topLength, rightLength, bottomLength);
	//RectF deltaMargin = margin - newMargin;
	//margin = newMargin;
	//
	//RectF newRect = GetRect();
	//newRect.MoveSidesLocal(-deltaMargin);
	//
	//SetRect(newRect);
}

inline void Widget::SetPadding(float leftLength, float topLength, float rightLength, float bottomLength)
{
	padding = RectF(leftLength, topLength, rightLength, bottomLength);

	//RectF newPadding = RectF(leftLength, topLength, rightLength, bottomLength);
	//RectF deltaPadding = padding - newPadding;
	//padding = newPadding;
	//
	//RectF newRect = GetRect();
	//newRect.MoveSidesLocal(-deltaPadding);
	//
	//SetRect(newRect);
}

inline void Widget::SetBorder(float leftLength, float topLength, float rightLength, float bottomLength, const Color& color)
{
	border = RectF(leftLength, topLength, rightLength, bottomLength);

	//RectF newBorder = RectF(leftLength, topLength, rightLength, bottomLength);
	//RectF deltaBorder = border - newBorder;
	//
	//border = newBorder;
	//borderColor = color;
	//
	//RectF newRect = GetRect();
	//newRect.MoveSidesLocal(-deltaBorder);
	//SetRect(newRect);
}