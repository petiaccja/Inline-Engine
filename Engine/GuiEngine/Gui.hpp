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

namespace inl::gui {

class GuiEngine;
class Gui;
class GuiText;
class GuiButton;
class GuiList;
class GuiSlider;
class GuiCollapsable;

enum class eGuiAlignHor
{
	NONE,
	LEFT,
	CENTER,
	RIGHT,
	STRETCH,
	STRETCH_LEFT,
	STRETCH_RIGHT,
	FILL_PARENT,
	FILL_PARENT_LEFT,
	FILL_PARENT_RIGHT,
	FIT_CHILDREN,
	FIT_CHILDREN_LEFT,
	FIT_CHILDREN_RIGHT,
};

enum class eGuiAlignVer
{
	NONE,
	TOP,
	CENTER,
	BOTTOM,
	STRETCH,
	STRETCH_TOP,
	STRETCH_BOTTOM,
	FILL_PARENT,
	FILL_PARENT_TOP,
	FILL_PARENT_BOTTOM,
	FIT_CHILDREN,
	FIT_CHILDREN_TOP,
	FIT_CHILDREN_BOTTOM,
};

class Gui
{
	friend class GuiEngine;
public:
	Gui();
	Gui(GuiEngine* guiEngine);
	Gui(GuiEngine* guiEngine, bool bLayer);

	virtual ~Gui() { Clear(); }
	void Clear();

	// Important to implement in derived classes
	virtual Gui* Clone() const { return new Gui(*this); }
	Gui& operator = (const Gui& other);

	template<class T>
	T* Add();

	void		Add(Gui* child);
	Gui*		AddGui();
	GuiText*	AddText();
	GuiButton*	AddButton();
	GuiList*	AddList();
	GuiSlider*	AddSlider();

	bool Remove(Gui* child);
	bool Remove();

	void TraverseTowardParents(const std::function<void(Gui*)>& fn);

	void Move(float dx, float dy);
	void Move(const Vector2f& delta) { Move(delta.x(), delta.y()); }

	const Vector2f& Measure(const Vector2f& availableSize);
	Vector2f Arrange(const Vector2f& pos, const Vector2f& size);
	Vector2f Arrange(float posX, float posY, const Vector2f& size) { return Arrange(Vector2f(posX, posY), size); }

	void EnableClipChildren() { SetClipChildren(true); }
	void DisableClipChildren() { SetClipChildren(false); }

	Gui*  AsPlane() { return (Gui*)this; }
	GuiText*   AsText() { return (GuiText*)this; }
	GuiButton* AsButton() { return (GuiButton*)this; }
	GuiList*   AsList() { return (GuiList*)this; }
	GuiSlider* AsSlider() { return (GuiSlider*)this; }
	GuiCollapsable* AsCollapsable() { return (GuiCollapsable*)this; }


	void SetClientRect(float x, float y, float width, float height) { SetClientRect(x, y, width, height, true); }
	void SetClientRect(const RectF rect) { SetClientRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), true); }

	void SetRect(float x, float y, float width, float height) { SetRect(x, y, width, height, true); }
	void SetRect(const RectF& rect) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), true); }

	void SetName(const std::wstring& str) { name = str; }
	void SetName(const std::string& str) { SetName(std::wstring(str.begin(), str.end())); }
	void SetContextMenu(Gui* c) { contextMenu = c; }
	void SetPos(const Vector2f& p) { SetPos(p.x(), p.y()); }
	void SetPos(float x, float y) { SetRect(x, y, size.x(), size.y()); }
	void SetCenterPos(float x, float y) { SetPos(x - GetHalfWidth(), y + GetHalfHeight()); }
	void SetPosX(float x) { SetRect(x, pos.y(), size.x(), size.y()); }
	void SetPosY(float y) { SetRect(pos.x(), y, size.x(), size.y()); }

	void SetWidth(float w) { SetSize(Vector2f(w, size.y())); }
	void SetHeight(float h) { SetSize(Vector2f(size.x(), h)); }

	void SetSize(const Vector2f& s) { SetSize(s.x(), s.y(), true); }
	void SetSize(float width, float height) { SetRect(pos.x(), pos.y(), width, height, true); }

	void SetClientSize(float width, float height) { SetClientRect(GetClientRect().left, GetClientRect().top, width, height, true); }
	void SetClientSize(const Vector2f& s) { SetClientSize(s.x(), s.y(), true); }

	void SetEventPropagationPolicy(eEventPropagationPolicy e) { eventPropagationPolicy = e; }

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
	void SetMargin(float length) { SetMargin(length, length, length, length); }

	void SetPadding(float leftLength, float topLength, float rightLength, float bottomLength);
	void SetPadding(float length) { SetPadding(length, length, length, length); }

	void SetAlign(eGuiAlignHor horizontalAlign, eGuiAlignVer verticalAlign) { this->horizontalAlign = horizontalAlign; this->verticalAlign = verticalAlign; bDirtyLayout = true; }
	void SetAlignHor(eGuiAlignHor align) { horizontalAlign = align; bDirtyLayout = true; }
	void SetAlignVer(eGuiAlignVer align) { verticalAlign = align; bDirtyLayout = true; }

	void AlignLeft()					{ SetAlignHor(eGuiAlignHor::LEFT); }
	void AlignRight()					{ SetAlignHor(eGuiAlignHor::RIGHT); }
	void AlignStretchHor()				{ SetAlignHor(eGuiAlignHor::STRETCH); }
	void AlignTop()						{ SetAlignVer(eGuiAlignVer::TOP); }
	void AlignBottom()					{ SetAlignVer(eGuiAlignVer::BOTTOM); }
	void AlignStretchVer()				{ SetAlignVer(eGuiAlignVer::STRETCH); }
	void AlignTopLeft()					{ SetAlign(eGuiAlignHor::LEFT, eGuiAlignVer::TOP); }
	void AlignTopRight()				{ SetAlign(eGuiAlignHor::RIGHT, eGuiAlignVer::TOP); }
	void AlignBottomLeft()				{ SetAlign(eGuiAlignHor::LEFT, eGuiAlignVer::BOTTOM); }
	void AlignBottomRight()				{ SetAlign(eGuiAlignHor::RIGHT, eGuiAlignVer::BOTTOM); }
	void AlignCenter()					{ SetAlign(eGuiAlignHor::CENTER, eGuiAlignVer::CENTER); }
	void AlignStretch()					{ SetAlign(eGuiAlignHor::STRETCH, eGuiAlignVer::STRETCH); }
	void AlignFillParentHor()			{ SetAlignHor(eGuiAlignHor::FILL_PARENT); }
	void AlignFillParentVer()			{ SetAlignVer(eGuiAlignVer::FILL_PARENT); }
	void AlignFillParent()				{ SetAlign(eGuiAlignHor::FILL_PARENT, eGuiAlignVer::FILL_PARENT); }
	void AlignFitChildrenHor()			{ SetAlignHor(eGuiAlignHor::FIT_CHILDREN); }
	void AlignFitChildrenVer()			{ SetAlignVer(eGuiAlignVer::FIT_CHILDREN); }
	void AlignFitChildren()				{ SetAlign(eGuiAlignHor::FIT_CHILDREN, eGuiAlignVer::FIT_CHILDREN); }

	void SetBgImageVisibility(bool bVisible) { bBgImageVisible = bVisible; }
	void SetBgColorVisibility(bool bVisible) { bBgColorVisible = bVisible; }

	//void SetAutoSize(bool bAutoWidth, bool bAutoHeight) { bAutoWidth = bAutoWidth; bAutoHeight = bAutoHeight; bDirtyLayout = true; }
	//void SetAutoSize(bool b) { SetAutoSize(b, b); }
	//void SetAutoWidth(bool b) { bAutoWidth = b; bDirtyLayout = true; }
	//void SetAutoHeight(bool b) { bAutoHeight = b; bDirtyLayout = true; }

	void HideBgImage() { SetBgImageVisibility(false); }
	void HideBgColor() { SetBgColorVisibility(false); }

	void ShowBgImage() { SetBgImageVisibility(true); }
	void ShowBgColor() { SetBgColorVisibility(true); }

	float GetClientSpaceCursorPosX();
	float GetClientSpaceCursorPosY();

	float GetPosX() { return pos.x(); }
	float GetPosY() { return pos.y(); }
	const Vector2f& GetPos() { return pos; }
	float GetCenterPosX() { return pos.x() + GetHalfWidth(); }
	float GetCenterPosY() { return pos.y() + GetHalfHeight(); }
	Vector2f GetCenterPos() { return pos + GetHalfSize(); }
	const Vector2f& GetSize() { return size; }
	float GetWidth() { return size.x(); }
	float GetHeight() { return size.y(); }
	float GetHalfWidth() { return GetWidth() * 0.5f; }
	float GetHalfHeight() { return GetHeight() * 0.5f; }
	Vector2f GetHalfSize() { return Vector2f(GetHalfWidth(), GetHalfHeight()); }

	const RectF& GetPadding() const { return padding; }
	const RectF& GetMargin() const { return margin; }

	float GetClientPosX() { return GetClientRect().left; }
	float GetClientPosY() { return GetClientRect().top; }
	Vector2f GetClientPos() { return GetClientRect().GetPos(); }
	float GetClientCenterPosY() { return GetClientPosY() - GetClientHalfHeight(); }
	float GetClientCenterPosX() { return GetClientPosX() + GetClientHalfWidth(); }
	Vector2f GetClientCenterPos() { return GetClientPos() + GetClientHalfSize(); }
	Vector2f GetClientSize() { return GetClientRect().GetSize(); }
	float GetClientWidth() { return GetClientRect().GetWidth(); }
	float GetClientHeight() { return GetClientRect().GetHeight(); }
	float GetClientHalfWidth() { return GetClientWidth() * 0.5f; }
	float GetClientHalfHeight() { return GetClientWidth() * 0.5f; }
	Vector2f GetClientHalfSize() { return GetClientSize() * 0.5f; }

	RectF GetRect();
	RectF GetClientRect();
	RectF GetPaddingRect();
	RectF GetBorderRect();

	Gui* GetParent() { return parent; }
	Gui* GetContextMenu() { return contextMenu; }
	std::vector<Gui*>& GetChildren() { return children; }
	RectF GetChildrenBoundRect();

	template<class T>
	T* GetChildByIdx(int index) { return (T*)GetChildren()[index]; }

	Gui* GetChildByIdx(int index) { return GetChildren()[index]; }
	int GetIndexInParent() { return indexInParent; }

	eEventPropagationPolicy GetEventPropagationPolicy() { return eventPropagationPolicy; }

	Color& GetBgActiveColor() { return bgActiveColor; }
	Color& GetBgIdleColor() { return bgIdleColor; }
	Color& GetBgHoverColor() { return bgHoverColor; }

	Gdiplus::Bitmap* GetBgActiveImage() { return bgActiveImage; }
	Gdiplus::Bitmap* GetBgIdleImage() { return bgIdleImage; }
	Gdiplus::Bitmap* GetBgHoverImage() { return bgHoverImage; }


	bool IsPointInside(Vector2i pt) { return GetRect().IsPointInside(pt); }

	bool IsLayer() { return bLayer; }
	bool IsChildrenClipEnabled() { return bClipChildren; }

	const Color& GetBorderColor() { return borderColor; }
	RectF GetBorder() { return border; }

protected:
	void SetClientRect(float x, float y, float width, float height, bool bFireEvents);
	void SetClientRect(const RectF rect, bool bFireEvents) { SetClientRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bFireEvents); }

	void SetRect(float x, float y, float width, float height, bool bFireEvents);
	void SetRect(const RectF& rect, bool bFireEvents) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bFireEvents); }
	
	void SetClientSize(float width, float height, bool bFireEvents) { SetClientRect(GetClientRect().left, GetClientRect().top, width, height, bFireEvents); }
	void SetClientSize(const Vector2f& s, bool bFireEvents) { SetClientSize(s.x(), s.y(), bFireEvents); }

	void SetSize(const Vector2f& s, bool bFireEvents) { SetSize(s.x(), s.y(), bFireEvents); }
	void SetSize(float width, float height, bool bFireEvents) { SetRect(pos.x(), pos.y(), width, height, bFireEvents); }

	void SetActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }

	void RefreshLayout();

	virtual Vector2f MeasureChildren(const Vector2f& availableSize);
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize);

	virtual void OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect);

protected:
	// Name it however you want
	std::wstring name;

	// Position
	Vector2f pos;

	// Width, Height
	Vector2f size;

	// Parent widget
	Gui* parent;

	// IsLayer ?
	bool bLayer;

	// Is clipped by parent client area?
	bool bClipChildren;

	// If true parent rectangle will be exactly aroound childs always
	//bool bAutoWidth;
	//bool bAutoHeight;

	// children index in parent
	int indexInParent;

	// Children widgets
	std::vector<Gui*> children;

	// What this widget should do with the message it reaches while moving up in hierarchy (toward parents)
	eEventPropagationPolicy eventPropagationPolicy;

	// If true, RefreshLayout() will be called before render, to ReArrange the layout as necessary
	bool bDirtyLayout;

	// Layering (Our neighbours in the tree hierarchy)
	Gui* front;
	Gui* back;

	// Attached context menu
	Gui* contextMenu;

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

	// Content align
	eGuiAlignVer verticalAlign;
	eGuiAlignHor horizontalAlign;

	// Access to god
	GuiEngine* guiEngine;

public:
	Vector2f DesiredSize;

	// Public events
	Delegate<void(CursorEvent& evt)> onMouseClicked;
	Delegate<void(CursorEvent& evt)> onMousePressed;
	Delegate<void(CursorEvent& evt)> onMouseReleased;
	Delegate<void(CursorEvent& evt)> onMouseMoved;
	Delegate<void(CursorEvent& evt)> onMouseEntered;
	Delegate<void(CursorEvent& evt)> onMouseLeaved;
	Delegate<void(CursorEvent& evt)> onMouseHovered;
	Delegate<void(float deltaTime)> onUpdate;

	Delegate<void(RectF& rect)> onTransformChanged;
	Delegate<void(Vector2f pos)> onPosChanged;
	Delegate<void(Vector2f size)> onSizeChanged;

	Delegate<void(RectF& rect)> onParentTransformChanged;
	Delegate<void(RectF& rect)> onChildTransformChanged;
	Delegate<void(Gui* parent)> onParentChanged;
	Delegate<void(Gui* child)> onChildAdded;
	Delegate<void(Gui* child)> onChildRemoved;
};

inline Gui::Gui(GuiEngine* guiEngine, bool bLayer)
{
	this->guiEngine = guiEngine;
	pos = Vector2f(0, 0);
	eventPropagationPolicy = eEventPropagationPolicy::PROCESS;
	size = Vector2f(60, 20);
	indexInParent = -1;
	this->bLayer = bLayer;
	parent = nullptr;
	front = nullptr;
	back = nullptr;
	contextMenu = nullptr;
	bgIdleColor = 45;
	bgHoverColor = 75;
	bgActiveImage = nullptr;
	bgIdleImage = nullptr;
	bgHoverImage = nullptr;
	border = RectF(0, 0, 0, 0);
	borderColor = Color(128);
	bBgImageVisible = true;
	bBgColorVisible = true;
	bClipChildren = false;
	margin = RectF(0, 0, 0, 0);
	padding = RectF(0, 0, 0, 0);
	DesiredSize = Vector2f(0, 0);
	bDirtyLayout = false;
	horizontalAlign = eGuiAlignHor::NONE;
	verticalAlign = eGuiAlignVer::NONE;
	

	SetBgActiveColor(bgIdleColor);

	onMouseEntered += [this](CursorEvent& event)
	{
		SetBgActiveColor(GetBgHoverColor());
		SetBgActiveImage(GetBgHoverImage());
	};

	onMouseLeaved += [this](CursorEvent& event)
	{
		SetBgActiveColor(GetBgIdleColor());
		SetBgActiveImage(GetBgIdleImage());
	};

	onSizeChanged += [this](Vector2f size)
	{
		bDirtyLayout = true;
	};
	
	onChildRemoved += [this](Gui* child)
	{
		bDirtyLayout = true;
	};
	
	onChildAdded += [this](Gui* child)
	{
		bDirtyLayout = true;
	};
}

inline Gui::Gui(GuiEngine* guiEngine)
:Gui(guiEngine, false)
{

}

inline Gui::Gui()
: Gui(nullptr, false)
{

}

inline void Gui::Clear()
{
	Remove();

	front = nullptr;
	back = nullptr;
	parent = nullptr;
	indexInParent = -1;

	for (Gui* c : children)
		delete c;

	children.clear();
}

inline Gui& Gui::operator = (const Gui& other)
{
	Clear();

	guiEngine = other.guiEngine;
	pos = other.pos;
	size = other.size;
	name = other.name;
	indexInParent = other.indexInParent;
	bLayer = other.bLayer;
	bClipChildren = other.bClipChildren;
	indexInParent = other.indexInParent;
	borderColor = other.borderColor;
	border = other.border;
	margin = other.margin;
	padding = other.padding;
	bDirtyLayout = other.bDirtyLayout;
	verticalAlign = other.verticalAlign;
	horizontalAlign = other.horizontalAlign;

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
	if (other.contextMenu)
		contextMenu = other.contextMenu->Clone();

	for (Gui* child : other.children)
		Add(child->Clone());

	// We are root, so attach to other's parent..
	if (other.parent && other.parent->IsLayer())
		other.parent->Add(this);

	return *this;
}

template<class T>
T* Gui::Add()
{
	T* child = new T(guiEngine);
	Add(child);
	return child;
}

inline void Gui::Add(Gui* child)
{
	if (child->parent)
		child->parent->Remove(child);

	child->parent = this;

	if (children.size() != 0)
	{
		Gui* lastControl = children[children.size() - 1];
		child->back = lastControl;
		lastControl->front = child;
	}

	child->indexInParent = children.size();
	children.push_back(child);

	child->onParentChanged(this);
	onChildAdded(child);
}

inline bool Gui::Remove(Gui* child)
{
	int potentialIndex = child->indexInParent;
	std::vector<Gui*>& children = GetChildren();

	if (potentialIndex < children.size())
	{
		Gui* potentialMatch = children[potentialIndex];

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

			onChildRemoved(child);
			child->onParentChanged(nullptr);

			return true;
		}
	}
	return false;
}

inline bool Gui::Remove()
{
	if (parent)
		return parent->Remove(this);

	return false;
}

inline void Gui::TraverseTowardParents(const std::function<void(Gui*)>& fn)
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

inline void Gui::Move(float dx, float dy)
{
	SetPos(pos.x() + dx, pos.y() + dy);
}

inline void Gui::SetRect(float x, float y, float width, float height, bool bFireEvents)
{
	RectF oldRect = GetRect();

	pos.x() = x;
	pos.y() = y;
	size.x() = width;
	size.y() = height;
	RectF rect = GetRect();

	if (rect != oldRect)
	{
		for (Gui* child : children)
		{
			child->Move(rect.GetPos() - oldRect.GetPos());

			child->onParentTransformChanged(rect);
		}

		if (bFireEvents)
		{
			onTransformChanged(rect);

			if (parent)
				parent->onChildTransformChanged(rect);
		}
	}

	if (bFireEvents)
	{
		if (rect.GetPos() != oldRect.GetPos())
		{
			onPosChanged(rect.GetPos());
		}

		if (rect.GetSize() != oldRect.GetSize())
		{
			onSizeChanged(rect.GetSize());
		}
	}
}

inline void Gui::SetClientRect(float x, float y, float width, float height, bool bFireEvents)
{
	RectF resultRect = RectF(x, y, x + width, y + height);

	// Convert length to signed offset
	RectF padding_ = padding;
	RectF border_ = border;
	//RectF margin_ = margin;
	padding_.left *= -1;
	padding_.top *= -1;
	border_.left *= -1;
	border_.top *= -1;
	//margin_.left *= -1;
	//margin_.top *= -1;

	// Now we can move the sides in appropriate directions
	resultRect.MoveSides(padding_);
	resultRect.MoveSides(border_);
	//resultRect.MoveSides(margin_);

	SetRect(resultRect, bFireEvents);
}

inline void Gui::SetBgIdleImage(const std::wstring& str)
{
	Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(str.c_str());

	if (bgIdleImage == GetBgActiveImage())
		SetBgActiveImage(newBitmap);

	if (bgIdleImage)
		delete bgIdleImage;

	bgIdleImage = newBitmap;
}

inline void Gui::SetBgHoverImage(const std::wstring& str)
{
	Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(str.c_str());

	if (bgHoverImage == GetBgActiveImage())
		SetBgActiveImage(newBitmap);

	if (bgHoverImage)
		delete bgHoverImage;

	bgHoverImage = newBitmap;
}

inline void Gui::SetBgIdleColor(const Color& color)
{
	if (bgIdleColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgIdleColor = color;
	ShowBgColor();
}

inline void Gui::SetBgToColor(const Color& idleColor, const Color& hoverColor)
{
	// Disable Image !
	HideBgImage();

	SetBgHoverColor(hoverColor);
	SetBgIdleColor(idleColor);
	ShowBgColor();
}

inline void Gui::SetBgHoverColor(const Color& color)
{
	if (bgHoverColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgHoverColor = color;
	ShowBgColor();
}

inline void Gui::SetBgActiveColorToIdle()
{
	SetBgActiveColor(GetBgIdleColor());
	ShowBgColor();
}

inline void Gui::SetBgActiveColorToHover()
{
	SetBgActiveColor(GetBgHoverColor());
	ShowBgColor();
}

inline void Gui::SetBgColorForAllStates(const Color& color)
{
	SetBgHoverColor(color);
	SetBgIdleColor(color);
	ShowBgColor();
}

inline void Gui::SetBgActiveImageToIdle()
{
	SetBgActiveImage(bgIdleImage);
}

inline void Gui::SetBgActiveImageToHover()
{
	SetBgActiveImage(bgHoverImage);
}

inline void Gui::SetBgImageForAllStates(const std::wstring& filePath)
{
	SetBgIdleImage(filePath);
	SetBgHoverImage(filePath);
}

inline void Gui::SetMargin(float leftLength, float topLength, float rightLength, float bottomLength)
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

inline void Gui::SetPadding(float leftLength, float topLength, float rightLength, float bottomLength)
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

inline void Gui::SetBorder(float leftLength, float topLength, float rightLength, float bottomLength, const Color& color)
{
	border = RectF(leftLength, topLength, rightLength, bottomLength);
	borderColor = color;
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

inline void Gui::RefreshLayout()
{
	Gui* arrangeRoot = nullptr;
	
	if (parent && !parent->IsLayer())
		arrangeRoot = parent;

	while (arrangeRoot && arrangeRoot->GetParent() && !arrangeRoot->IsLayer())
		arrangeRoot = arrangeRoot->GetParent();

	if (!arrangeRoot || arrangeRoot->IsLayer())
		arrangeRoot = this;

	// The size children wants
	Vector2f desiredSize = arrangeRoot->Measure(arrangeRoot->GetClientSize());

	Vector2f finalSize = desiredSize;// Vector2f::Max(desiredSize, GetClientSize());

	// Do we really want to always arrange to parent's position?
	arrangeRoot->Arrange(arrangeRoot->GetPos(), finalSize);
}

inline const Vector2f& Gui::Measure(const Vector2f& availableSize)
{
	Vector2f avaSize = availableSize;
	//avaSize.x() -= margin.left + margin.right;
	//avaSize.y() -= margin.top + margin.bottom;

	DesiredSize = MeasureChildren(avaSize);


	// Variations...
	//1. FIT & PARENT->FIT
	//2. FIT & PARENT_>FILL
	//3. FILL & parent->FIT
	//4. FILL & parent-> FILL

	// TEST PURPOSE, Force user chosed size as desired size
	if (horizontalAlign != eGuiAlignHor::FIT_CHILDREN && horizontalAlign != eGuiAlignHor::FILL_PARENT)
		DesiredSize.x() = GetSize().x();

	if (verticalAlign != eGuiAlignVer::FIT_CHILDREN && verticalAlign != eGuiAlignVer::FILL_PARENT)
		DesiredSize.y() = GetSize().y();

	// Fit stretch to parent
	if (parent)
	{
		if (horizontalAlign == eGuiAlignHor::FILL_PARENT && parent->horizontalAlign != eGuiAlignHor::FIT_CHILDREN)
		{
			DesiredSize.x() = std::max(DesiredSize.x(), parent->GetClientSize().x());
		}
		if (verticalAlign == eGuiAlignVer::FILL_PARENT && parent->verticalAlign != eGuiAlignVer::FIT_CHILDREN)
		{
			DesiredSize.y() = std::max(DesiredSize.y(), parent->GetClientSize().y());
		}
	}

	//DesiredSize.x() += margin.left + margin.right;
	//DesiredSize.y() += margin.top + margin.bottom;
	return DesiredSize;
}

inline Vector2f Gui::Arrange(const Vector2f& pos, const Vector2f& size)
{
	Vector2f newPos = pos;
	Vector2f newSize = size;

	// Self alignment in parent
	if (parent)
	{
		RectF contentRect = parent->GetClientRect();
	
		switch (horizontalAlign)
		{
		case eGuiAlignHor::LEFT:
		{
			newPos.x() = contentRect.left;
			break;
		}
		case eGuiAlignHor::CENTER:
		{
			newPos.x() = contentRect.GetCenter().x() - newSize.x() * 0.5;
			break;
		}
		case eGuiAlignHor::RIGHT:
		{
			newPos.x() = contentRect.right - newSize.x();
			break;
		}
		case eGuiAlignHor::STRETCH:
		{
			// ???
			break;
		}
		case eGuiAlignHor::FILL_PARENT:
		{
			newPos.x() = contentRect.left;
			break;
		}
		}
	}
	

	// Child content can only modify our size if we have autoWidth or autoHeight set !
	//if (bAutoWidth)
	//	newSize.x() = size.x();
	//else
	//	newSize.x() = DesiredSize.x();
	//
	//if (bAutoHeight)
	//	newSize.y() = size.y();
	//else
	//	newSize.y() = DesiredSize.y();


	//newPos.x() += margin.left;
	//newPos.y() += margin.top;
	//newSize.x() -= margin.left + margin.right;
	//newSize.y() -= margin.top + margin.bottom;

	// Arrange ourselves
	SetRect(newPos.x(), newPos.y(), newSize.x(), newSize.y(), false);

	// Arrange children
	Vector2f finalSize = ArrangeChildren(newSize);

	// Deferred layout refresh
	bDirtyLayout = false;

	return finalSize;
}

inline Vector2f Gui::MeasureChildren(const Vector2f& availableSize)
{
	Vector2f size(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f desiredSize = child->Measure(availableSize);

		size = Vector2f::Max(size, desiredSize);
	}
	return size;
}

inline Vector2f Gui::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f size(0, 0);
	for (Gui* child : GetChildren())
	{
		child->Arrange(child->GetPos(), child->DesiredSize);
	}
	return finalSize;
}

inline void Gui::OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect)
{
	if (bDirtyLayout)
		RefreshLayout();

	RectF paddingRect = GetPaddingRect();
	RectF borderRect = GetBorderRect();

	Gdiplus::Rect gdiBorderRect(borderRect.left, borderRect.top, borderRect.GetWidth(), borderRect.GetHeight());
	Gdiplus::Rect gdiPaddingRect(paddingRect.left, paddingRect.top, paddingRect.GetWidth(), paddingRect.GetHeight());
	Gdiplus::Rect gdiClipRect(clipRect.left, clipRect.top, clipRect.GetWidth(), clipRect.GetHeight());

	// Clipping
	graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeReplace);

	// Draw left border
	Gdiplus::SolidBrush borderBrush(Gdiplus::Color(borderColor.a, borderColor.r, borderColor.g, borderColor.b));
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
	if (GetBgActiveImage() && bBgImageVisible)
	{
		graphics->DrawImage(GetBgActiveImage(), gdiPaddingRect);
	}
	else if (bBgColorVisible) // Draw Background Colored Rectangle
	{
		Color bgColor = GetBgActiveColor();
		Gdiplus::SolidBrush brush(Gdiplus::Color(bgColor.a, bgColor.r, bgColor.g, bgColor.b));
		graphics->FillRectangle(&brush, gdiPaddingRect);
	}
}

inline RectF Gui::GetChildrenBoundRect()
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

inline RectF Gui::GetRect()
{
	return RectF::FromSize(pos.x(), pos.y(), size.x(), size.y());
}

inline RectF Gui::GetClientRect()
{
	RectF result = GetRect();

	//result.MoveSidesLocal(-margin);
	result.MoveSidesLocal(-border);
	result.MoveSidesLocal(-padding);

	return result;
}

inline RectF Gui::GetPaddingRect()
{
	RectF result = GetRect();

	//result.MoveSidesLocal(-margin);
	result.MoveSidesLocal(-border);

	return result;
}

inline RectF Gui::GetBorderRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-margin);

	return result;
}

} // namespace inl::gui