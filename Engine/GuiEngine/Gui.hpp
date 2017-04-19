#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiEvent.h"

// TMP HEKK (REMOVE)
#include <BaseLibrary\Platform\Window.hpp>
//#define min(a,b) a < b ? a . b
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
};

enum class eGuiAlignVer
{
	NONE,
	TOP,
	CENTER,
	BOTTOM,
};

enum class eGuiStretch
{
	NONE,
	FILL_PARENT,
	FIT_CHILDREN,
};

class Gui
{
	friend class GuiEngine;
public:
	Gui();
	Gui(GuiEngine* guiEngine);
	Gui(GuiEngine* guiEngine, bool bLayer);

	void InitFromImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath);

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
	GuiCollapsable* AddCollapsable();

	bool Remove(Gui* child);
	bool Remove();

	void TraverseTowardParents(const std::function<void(Gui*)>& fn);

	void Move(float dx, float dy);
	void Move(const Vector2f& delta) { Move(delta.x(), delta.y()); }

	const Vector2f& Measure(const Vector2f& availableSize);

	//Vector2f Arrange(const Vector2f& pos, const Vector2f& size) { Arrange(pos, size, true); }
	Vector2f Arrange(const Vector2f& pos, const Vector2f& size, bool bEnableFillParent = true);
	Vector2f Arrange(float posX, float posY, const Vector2f& size) { return Arrange(Vector2f(posX, posY), size, true); }	

	void EnableClipChildren() { SetClipChildren(true); }
	void DisableClipChildren() { SetClipChildren(false); }

	Gui*  AsPlane() { return (Gui*)this; }
	GuiText*   AsText() { return (GuiText*)this; }
	GuiButton* AsButton() { return (GuiButton*)this; }
	GuiList*   AsList() { return (GuiList*)this; }
	GuiSlider* AsSlider() { return (GuiSlider*)this; }
	GuiCollapsable* AsCollapsable() { return (GuiCollapsable*)this; }


	void SetContentRect(float x, float y, float width, float height) { SetContentRect(x, y, width, height, true, true); }
	void SetContentRect(const RectF rect) { SetContentRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()); }

	void SetRect(float x, float y, float width, float height) { SetRect(x, y, width, height, true, true); }
	void SetRect(const RectF& rect) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()); }

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

	void SetSize(const Vector2f& s) { SetSize(s.x(), s.y()); }
	void SetSize(float width, float height) { SetRect(pos.x(), pos.y(), width, height, true, true); }

	void SetContentSize(float width, float height) { SetContentRect(GetContentRect().left, GetContentRect().top, width, height, true, true); }
	void SetContentSize(const Vector2f& s) { SetContentSize(s.x(), s.y()); }

	void SetEventPropagationPolicy(eEventPropagationPolicy e) { eventPropagationPolicy = e; }

	void SetClipChildren(bool b) { bClipChildren = b; }

	void SetBgToColor(const Color& idleColor, const Color& hoverColor);
	void SetBgIdleColor(const Color& color);
	void SetBgHoverColor(const Color& color);
	void SetBgActiveColor(const Color& color) { bgActiveColor = color; }
	void SetBgActiveColorToIdle();
	void SetBgActiveColorToHover();
	void SetBgColorForAllStates(const Color& color);

	void SetBgToImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath);
	void SetBgIdleImage(const std::wstring& path);
	void SetBgHoverImage(const std::wstring& path);
	void SetBgActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }
	void SetBgActiveImageToIdle();
	void SetBgActiveImageToHover();
	void SetBgImageForAllStates(const std::wstring& filePath);

	void SetBorder(float leftLength, float rightLength, float topLength, float bottomLength, const Color& color);
	void SetBorder(float borderLength, const Color& color) { SetBorder(borderLength, borderLength, borderLength, borderLength, color); }

	void SetMargin(float leftLength, float topLength, float rightLength, float bottomLength);
	void SetMargin(float length) { SetMargin(length, length, length, length); }

	void SetMarginLeft(float length)	{ SetMargin(length, margin.top, margin.right, margin.bottom); }
	void SetMarginRight(float length)	{ SetMargin(margin.left, margin.top, length, margin.bottom); }
	void SetMarginTop(float length)		{ SetMargin(margin.left, length, margin.right, margin.bottom); }
	void SetMarginBottom(float length)	{ SetMargin(margin.left, margin.top, margin.right, length); }

	void SetPadding(float leftLength, float topLength, float rightLength, float bottomLength);
	void SetPadding(float length) { SetPadding(length, length, length, length); }

	void SetAlign(eGuiAlignHor horizontalAlign, eGuiAlignVer verticalAlign) { alignHor = horizontalAlign; alignVer = verticalAlign; bDirtyLayout = true; }
	void SetAlignHor(eGuiAlignHor align) { alignHor = align; bDirtyLayout = true; }
	void SetAlignVer(eGuiAlignVer align) { alignVer = align; bDirtyLayout = true; }

	void SetStretch(eGuiStretch stretch) { SetStretch(stretch, stretch); }
	void SetStretch(eGuiStretch horizontalStretch, eGuiStretch verticalStretch) { stretchHor = horizontalStretch; stretchVer = verticalStretch; bDirtyLayout = true; }
	void SetStretchHor(eGuiStretch stretch) { stretchHor = stretch; bDirtyLayout = true; }
	void SetStretchVer(eGuiStretch stretch) { stretchVer = stretch; bDirtyLayout = true; }

	void AlignLeft()					{ SetAlignHor(eGuiAlignHor::LEFT); }
	void AlignRight()					{ SetAlignHor(eGuiAlignHor::RIGHT); }
	void AlignTop()						{ SetAlignVer(eGuiAlignVer::TOP); }
	void AlignBottom()					{ SetAlignVer(eGuiAlignVer::BOTTOM); }
	void AlignTopLeft()					{ SetAlign(eGuiAlignHor::LEFT, eGuiAlignVer::TOP); }
	void AlignTopRight()				{ SetAlign(eGuiAlignHor::RIGHT, eGuiAlignVer::TOP); }
	void AlignBottomLeft()				{ SetAlign(eGuiAlignHor::LEFT, eGuiAlignVer::BOTTOM); }
	void AlignBottomRight()				{ SetAlign(eGuiAlignHor::RIGHT, eGuiAlignVer::BOTTOM); }
	void AlignCenter()					{ SetAlign(eGuiAlignHor::CENTER, eGuiAlignVer::CENTER); }
	void AlignCenterHor() { SetAlignHor(eGuiAlignHor::CENTER); }
	void AlignCenterVer() { SetAlignVer(eGuiAlignVer::CENTER); }

	void StretchNone() { SetStretch(eGuiStretch::NONE); }
	void StretchNoneHor() { SetStretchHor(eGuiStretch::NONE); }
	void StretchNoneVor() { SetStretchVer(eGuiStretch::NONE); }
	
	void StretchFillParent() { SetStretch(eGuiStretch::FILL_PARENT); }
	void StretchFillParentHor() { SetStretchHor(eGuiStretch::FILL_PARENT); }
	void StretchFillParentVer() { SetStretchVer(eGuiStretch::FILL_PARENT); }

	void StretchFitToChildren() { SetStretch(eGuiStretch::FIT_CHILDREN); }
	void StretchFitToChildrenHor() { SetStretchHor(eGuiStretch::FIT_CHILDREN); }
	void StretchFitToChildrenVer() { SetStretchVer(eGuiStretch::FIT_CHILDREN); }
	
	void SetBgImageVisibility(bool bVisible) { bBgImageVisible = bVisible; }
	void SetBgColorVisibility(bool bVisible) { bBgColorVisible = bVisible; }

	void HideBgImage() { SetBgImageVisibility(false); }
	void HideBgColor() { SetBgColorVisibility(false); }

	void ShowBgImage() { SetBgImageVisibility(true); }
	void ShowBgColor() { SetBgColorVisibility(true); }

	template<class T>
	T* Copy(T* other);

	float GetContentSpaceCursorPosX();
	float GetContentSpaceCursorPosY();

	float GetPosX() { return pos.x(); }
	float GetPosY() { return pos.y(); }
	const Vector2f& GetPos() { return pos; }
	float GetCenterPosX() { return pos.x() + GetHalfWidth(); }
	float GetCenterPosY() { return pos.y() + GetHalfHeight(); }
	Vector2f GetCenterPos() { return pos + GetHalfSize(); }
	const Vector2f& GetSize() { return size; }
	float GetSizeX() { return size.x(); }
	float GetSizeY() { return size.y(); }
	float GetWidth() { return size.x(); }
	float GetHeight() { return size.y(); }
	float GetHalfWidth() { return GetWidth() * 0.5f; }
	float GetHalfHeight() { return GetHeight() * 0.5f; }
	Vector2f GetHalfSize() { return Vector2f(GetHalfWidth(), GetHalfHeight()); }

	const RectF& GetPadding() const { return padding; }
	const RectF& GetMargin() const { return margin; }

	float GetContentPosX() { return GetContentPos().x(); }
	float GetContentPosY() { return GetContentPos().y(); }
	Vector2f GetContentPos() { return GetContentRect().GetPos(); }
	float GetContentCenterPosY() { return GetContentPosY() - GetContentHalfHeight(); }
	float GetContentCenterPosX() { return GetContentPosX() + GetContentHalfWidth(); }
	Vector2f GetContentCenterPos() { return GetContentPos() + GetContentHalfSize(); }
	Vector2f GetContentSize() { return GetContentRect().GetSize(); }
	float GetContentSizeX() { return GetContentSize().x(); }
	float GetContentSizeY() { return GetContentSize().y(); }
	float GetContentWidth() { return GetContentRect().GetWidth(); }
	float GetContentHeight() { return GetContentRect().GetHeight(); }
	float GetContentHalfWidth() { return GetContentWidth() * 0.5f; }
	float GetContentHalfHeight() { return GetContentWidth() * 0.5f; }
	Vector2f GetContentHalfSize() { return GetContentSize() * 0.5f; }

	RectF GetRect();
	RectF GetContentRect();
	RectF GetPaddingRect();
	RectF GetBorderRect();
	RectF GetChildrenRect();

	Gui* GetParent() { return parent; }
	Gui* GetContextMenu() { return contextMenu; }
	std::vector<Gui*>& GetChildren() { return children; }

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
	bool IsHovered() { return bHovered; }
	bool IsCursorInside();

	const Color& GetBorderColor() { return borderColor; }
	RectF GetBorder() { return border; }

protected:
	void SetContentRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty);
	void SetContentRect(const RectF rect, bool bMoveChildren, bool bMakeLayoutDirty) { SetContentRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bMoveChildren, bMakeLayoutDirty); }

	void SetRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty);
	void SetRect(const RectF& rect, bool bMoveChildren, bool bMakeLayoutDirty) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bMoveChildren, bMakeLayoutDirty); }
	
	void SetContentSize(float width, float height, bool bMakeLayoutDirty) { SetContentRect(GetContentRect().left, GetContentRect().top, width, height, false, bMakeLayoutDirty); }
	void SetContentSize(const Vector2f& s, bool bMakeLayoutDirty) { SetContentSize(s.x(), s.y(), bMakeLayoutDirty); }

	void SetSize(const Vector2f& s, bool bMakeLayoutDirty) { SetSize(s.x(), s.y(), bMakeLayoutDirty); }
	void SetSize(float width, float height, bool bMakeLayoutDirty) { SetRect(pos.x(), pos.y(), width, height, false, bMakeLayoutDirty); }

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

	// Is clipped by parent content area?
	bool bClipChildren;

	// Is hovered by cursor?
	bool bHovered;

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

	// Alignment & Stretch
	eGuiAlignHor	alignHor;
	eGuiAlignVer	alignVer;
	eGuiStretch		stretchHor;
	eGuiStretch		stretchVer;

	// Access to god
	GuiEngine* guiEngine;

public:
	Vector2f desiredSize;

	// Public events
	Delegate<void(CursorEvent& evt)> onMouseClicked;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseClickedClonable;

	Delegate<void(CursorEvent& evt)> onMouseDblClicked;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseDblClickedClonable;
	
	Delegate<void(CursorEvent& evt)> onMousePressed;
	Delegate<void(Gui* self, CursorEvent& evt)> onMousePressedClonable;

	Delegate<void(CursorEvent& evt)> onMouseReleased;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseReleasedClonable;

	Delegate<void(CursorEvent& evt)> onMouseMoved;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseMovedClonable;

	Delegate<void(CursorEvent& evt)> onMouseEntered;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseEnteredClonable;

	Delegate<void(CursorEvent& evt)> onMouseLeaved;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseLeavedClonable;

	Delegate<void(CursorEvent& evt)> onMouseHovered;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseHoveredClonable;

	Delegate<void(float deltaTime)> onUpdate;
	Delegate<void(Gui* self, float deltaTime)> onUpdateClonable;

	Delegate<void(RectF& rect)> onTransformChanged;
	Delegate<void(Gui* self, RectF& rect)> onTransformChangedClonable;

	Delegate<void(Vector2f pos)> onPosChanged;
	Delegate<void(Gui* self, Vector2f pos)> onPosChangedClonable;

	Delegate<void(Vector2f size)> onSizeChanged;
	Delegate<void(Gui* self, Vector2f size)> onSizeChangedClonable;

	Delegate<void(RectF& rect)> onParentTransformChanged;
	Delegate<void(Gui* self, RectF& rect)> onParentTransformChangedClonable;

	Delegate<void(RectF& rect)> onChildTransformChanged;
	Delegate<void(Gui* self, RectF& rect)> onChildTransformChangedClonable;

	Delegate<void(Gui* parent)> onParentChanged;
	Delegate<void(Gui* self, Gui* parent)> onParentChangedClonable;

	Delegate<void(Gui* child)> onChildAdded;
	Delegate<void(Gui* self, Gui* child)> onChildAddedClonable;

	Delegate<void(Gui* child)> onChildRemoved;
	Delegate<void(Gui* self, Gui* child)> onChildRemovedClonable;
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
	bClipChildren = true;
	margin = RectF(0, 0, 0, 0);
	padding = RectF(0, 0, 0, 0);
	desiredSize = Vector2f(0, 0);
	bDirtyLayout = false;
	alignHor = eGuiAlignHor::NONE;
	alignVer = eGuiAlignVer::NONE;
	stretchHor = eGuiStretch::NONE;
	stretchVer = eGuiStretch::NONE;
	bHovered = false;

	SetBgActiveColor(bgIdleColor);

	onMouseEnteredClonable += [](Gui* self, CursorEvent& event)
	{
		self->SetBgActiveColor(self->GetBgHoverColor());
		self->SetBgActiveImage(self->GetBgHoverImage());
		self->bHovered = true;
	};

	onMouseLeavedClonable += [](Gui* self, CursorEvent& event)
	{
		self->SetBgActiveColor(self->GetBgIdleColor());
		self->SetBgActiveImage(self->GetBgIdleImage());
		self->bHovered = false;
	};
	
	onChildRemovedClonable += [](Gui* self, Gui* child)
	{
		self->bDirtyLayout = true;
	};
	
	onChildAddedClonable += [](Gui* self, Gui* child)
	{
		self->bDirtyLayout = true;
	};
}

inline Gui::Gui(GuiEngine* guiEngine)
:Gui(guiEngine, false)
{

}

inline Gui::Gui()
:Gui(nullptr, false)
{

}

inline void Gui::Clear()
{
	//Remove();

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
	alignHor = other.alignHor;
	alignVer = other.alignVer;
	stretchHor = other.stretchHor;
	stretchVer = other.stretchVer;

	onMouseClickedClonable = other.onMouseClickedClonable;
	onMousePressedClonable = other.onMousePressedClonable;
	onMouseReleasedClonable = other.onMouseReleasedClonable;
	onMouseMovedClonable = other.onMouseMovedClonable;
	onMouseEnteredClonable = other.onMouseEnteredClonable;
	onMouseLeavedClonable = other.onMouseLeavedClonable;
	onMouseHoveredClonable = other.onMouseHoveredClonable;
	onUpdateClonable = other.onUpdateClonable;
	onTransformChangedClonable = other.onTransformChangedClonable;
	onPosChangedClonable = other.onPosChangedClonable;
	onSizeChangedClonable = other.onSizeChangedClonable;
	onParentTransformChangedClonable = other.onParentTransformChangedClonable;
	onChildTransformChangedClonable = other.onChildTransformChangedClonable;
	onParentChangedClonable = other.onParentChangedClonable;
	onChildAddedClonable = other.onChildAddedClonable;
	onChildRemovedClonable = other.onChildRemovedClonable;

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
	contextMenu = Copy(other.contextMenu);

	for (Gui* child : other.children)
		Add(child->Clone());

	// We are root, so attach to other's parent..
	if (other.parent && other.parent->IsLayer())
		other.parent->Add(this);

	return *this;
}

inline void Gui::InitFromImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath)
{
	SetBgToImage(idleImagePath, hoverImagePath);
	SetSize(Vector2f(GetBgIdleImage()->GetWidth(), GetBgIdleImage()->GetHeight()));
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
	child->onParentChangedClonable(child, this);

	onChildAdded(child);
	onChildAddedClonable(this, child);
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
			onChildRemovedClonable(this, child);

			child->onParentChanged(nullptr);
			child->onParentChangedClonable(child, nullptr);

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

template<class T>
T* Gui::Copy(T* other)
{
	if (!other)
		return nullptr;

	int idx = other->GetIndexInParent();
	if (idx == -1)
		return other->Clone();
	else
		return GetChildByIdx<T>(other->GetIndexInParent());
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

inline void Gui::SetRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty)
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
			if(bMoveChildren)
				child->Move(rect.GetPos() - oldRect.GetPos());

			child->onParentTransformChanged(rect);
			child->onParentTransformChangedClonable(child, rect);
		}
		
		onTransformChanged(rect);

		if (parent)
		{
			parent->onChildTransformChanged(rect);
			parent->onChildTransformChangedClonable(parent, rect);
		}
	}

	if (rect.GetPos() != oldRect.GetPos())
	{
		onPosChanged(rect.GetPos());
	}

	if (rect.GetSize() != oldRect.GetSize())
	{
		onSizeChanged(rect.GetSize());

		if (bMakeLayoutDirty)
			bDirtyLayout = true;
	}
}

inline void Gui::SetContentRect(float x, float y, float width, float height, bool bFireEvents, bool bMoveChildren)
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

	SetRect(resultRect, bFireEvents, bMoveChildren);
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
}

inline void Gui::SetBgToColor(const Color& idleColor, const Color& hoverColor)
{
	// Disable Image !
	HideBgImage();

	SetBgHoverColor(hoverColor);
	SetBgIdleColor(idleColor);
}

inline void Gui::SetBgHoverColor(const Color& color)
{
	if (bgHoverColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgHoverColor = color;
}

inline void Gui::SetBgActiveColorToIdle()
{
	SetBgActiveColor(GetBgIdleColor());
}

inline void Gui::SetBgActiveColorToHover()
{
	SetBgActiveColor(GetBgHoverColor());
}

inline void Gui::SetBgColorForAllStates(const Color& color)
{
	SetBgHoverColor(color);
	SetBgIdleColor(color);
}

inline void Gui::SetBgToImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath)
{
	HideBgColor();

	SetBgIdleImage(idleImagePath);
	SetBgHoverImage(hoverImagePath);
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
	//if (IsLayer())
	//	return;

	Gui* arrangeRoot = this;

	while (arrangeRoot && arrangeRoot->GetParent())
		arrangeRoot = arrangeRoot->GetParent();

	// How much space that control needs
	Vector2f desiredSize = arrangeRoot->Measure(arrangeRoot->GetContentSize());

	//desiredSize.x() += border.left + border.right;
	//desiredSize.y() += border.top + border.bottom;

	arrangeRoot->Arrange(arrangeRoot->GetPos(), desiredSize, true);
}

inline const Vector2f& Gui::Measure(const Vector2f& availableSize)
{
	Vector2f avaSize = availableSize;
	//avaSize.x() -= margin.left + margin.right;
	//avaSize.y() -= margin.top + margin.bottom;

	desiredSize = MeasureChildren(avaSize);

	//desiredSize.x() += margin.left + margin.right;
	//desiredSize.y() += margin.top + margin.bottom;
	return desiredSize;
}

inline Vector2f Gui::Arrange(const Vector2f& pos, const Vector2f& size, bool bEnableFillParent /*= true*/)
{
	Vector2f newPos = pos;
	Vector2f newSize = size;

	switch (alignVer)
	{
	case eGuiAlignVer::TOP:
	{
		newPos.y() = parent->GetPosY();
		break;
	}
	case eGuiAlignVer::CENTER:
	{
		if (parent)
		{
			newPos.y() = parent->GetCenterPosY() - newSize.y() * 0.5;
		}
		break;
	}
	case eGuiAlignVer::BOTTOM:
	{
		newPos.y() = parent->GetHeight() - newSize.y();
		break;
	}
	}

	switch (alignHor)
	{
	case eGuiAlignHor::LEFT:
	{
		newPos.x() = parent->GetPosX();
		break;
	}
	case eGuiAlignHor::CENTER:
	{
		if (parent)
		{
			newPos.x() = parent->GetCenterPosX() - newSize.x() * 0.5;
		}
		break;
	}
	case eGuiAlignHor::RIGHT:
	{
		newPos.x() = parent->GetWidth() - newSize.x();
		break;
	}
	}

	bool bFitToChildrenHor = stretchHor == eGuiStretch::FIT_CHILDREN;
	bool bFitToChildrenVer = stretchVer == eGuiStretch::FIT_CHILDREN;
	bool bFitToChildren = bFitToChildrenHor | bFitToChildrenVer;
	bool bFillParentHor = stretchHor == eGuiStretch::FILL_PARENT;
	bool bFillParentVer = stretchVer == eGuiStretch::FILL_PARENT;
	bool bFillParent = (bFillParentHor || bFillParentVer) && bEnableFillParent;

	if (bFitToChildren)
	{
		ArrangeChildren(newSize);

		RectF childrenRect = GetChildrenRect();
		
		if(bFitToChildrenHor)
			newSize.x() = childrenRect.right - newPos.x();

		if (bFitToChildrenVer)
			newSize.y() = childrenRect.bottom - newPos.y();
	}

	if (bFillParent)
	{
		if (bFillParentHor && parent->stretchHor != eGuiStretch::FIT_CHILDREN)
		{
			newSize.x() = parent->GetContentSizeX();
			newPos.x() = parent->GetContentPosX() + margin.left;
		}
		
		if (bFillParentVer && parent->stretchVer != eGuiStretch::FIT_CHILDREN)
		{
			newSize.y() = parent->GetContentSizeY();
			newPos.y() = parent->GetContentPosY() + margin.top;
		}
	}

	SetRect(newPos.x(), newPos.y(), newSize.x(), newSize.y(), true, false);

	ArrangeChildren(newSize);

	// Here we have up to date layout, it's not dirty
	bDirtyLayout = false;

	return newSize;
}

inline Vector2f Gui::MeasureChildren(const Vector2f& availableSize)
{
	Vector2f size = GetSize();
	for (Gui* child : GetChildren())
	{
		Vector2f desiredSize = child->Measure(availableSize);

		size = Vector2f::Max(size, desiredSize);
	}
	return size;
}

inline Vector2f Gui::ArrangeChildren(const Vector2f& finalSize)
{
	std::vector<Gui*>& children = GetChildren();

	//RectF finalRect(GetPos(), finalSize);

	Vector2f size(0, 0);
	for (Gui* child : GetChildren())
	{
		//RectF childRect = RectF::Intersect(finalRect, RectF(child->GetContentPos(), child->desiredSize));

		Vector2f sizeUsed = child->Arrange(child->GetPos(), child->desiredSize);
		//Vector2f sizeUsed = child->Arrange(childRect.GetPos(), childRect.GetSize());
		size = Vector2f::Max(size, sizeUsed);
	}

	return size;
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

inline RectF Gui::GetRect()
{
	return RectF(pos, size);
}

inline RectF Gui::GetContentRect()
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

inline RectF Gui::GetChildrenRect()
{
	auto& children = GetChildren();

	if (children.size() == 0)
		return RectF(0, 0, 0, 0);

	// TODO slow performance.. Collect child boundingbox when onChildAdded, onChildRemoved, onChildTransformChanged..
	RectF boundingRect = children[0]->GetRect();

	for (int i = 1; i < children.size(); ++i)
		boundingRect.Union(children[i]->GetRect());

	return boundingRect;
}

} // namespace inl::gui