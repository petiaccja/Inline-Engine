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
	FIT_TO_CHILDREN,
};

enum class eGuiDirection
{
	VERTICAL,
	HORIZONTAL,
};



class GuiEngine;
class Gui;
class GuiText;
class GuiButton;
class GuiList;
class GuiSlider;
class GuiCollapsable;
class GuiSplitter;

class Gui
{
	friend class GuiEngine;
public:
	Gui();
	Gui(GuiEngine* guiEngine);
	Gui(GuiEngine* guiEngine, bool bLayer);
	Gui(const Gui& other) { *this = other; }

	void InitFromImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath);

	virtual ~Gui() { Clear(); }
	void Clear();

	// Important to implement in derived classes
	virtual Gui* Clone() const { return new Gui(*this); }
	Gui& operator = (const Gui& other);

	template<class T>
	T* Add();

	void			Add(Gui* child) { Add(child, true); }

	Gui*			AddGui();
	GuiText*		AddText();
	GuiButton*		AddButton();
	GuiList*		AddList();
	GuiSlider*		AddSlider();
	GuiCollapsable* AddCollapsable();
	GuiSplitter*	AddSplitter();

	bool Remove(Gui* child) { return Remove(child, true); }
	bool Remove();

	void TraverseTowardParents(const std::function<void(Gui*)>& fn);

	void RefreshLayout();
	bool IsLayoutNeedRefresh() { return bLayoutNeedRefresh; }

	void Move(float dx, float dy);
	void Move(const Vector2f& delta) { Move(delta.x(), delta.y()); }

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
	GuiSplitter* AsSplitter() { return (GuiSplitter*)this; }


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

	void Stretch(eGuiStretch stretch) { Stretch(stretch, stretch); }
	void Stretch(eGuiStretch horizontalStretch, eGuiStretch verticalStretch) { stretchHor = horizontalStretch; stretchVer = verticalStretch; bLayoutNeedRefresh = true; }
	void StretchHor(eGuiStretch stretch) { stretchHor = stretch; bLayoutNeedRefresh = true; }
	void StretchVer(eGuiStretch stretch) { stretchVer = stretch; bLayoutNeedRefresh = true; }

	void StretchHorFillParent() {StretchHor(eGuiStretch::FILL_PARENT); }
	void StretchHorFitToChildren() { StretchHor(eGuiStretch::FIT_TO_CHILDREN); }
	void StretchHorNone() { StretchHor(eGuiStretch::NONE); }

	void StretchVerFillParent() { StretchVer(eGuiStretch::FILL_PARENT); }
	void StretchVerFitToChildren() { StretchVer(eGuiStretch::FIT_TO_CHILDREN); }
	void StretchVerNone() { StretchVer(eGuiStretch::NONE); }

	void StretchFillParent(bool bHor, bool bVer)
	{
		bool bWasHorFill = stretchHor == eGuiStretch::FILL_PARENT;
		bool bWasVerFill = stretchVer == eGuiStretch::FILL_PARENT;

		eGuiStretch hor = bHor ? eGuiStretch::FILL_PARENT : (bWasHorFill ? eGuiStretch::NONE : stretchHor);
		eGuiStretch ver = bVer ? eGuiStretch::FILL_PARENT : (bWasVerFill ? eGuiStretch::NONE : stretchVer);

		Stretch(hor, ver);
	}

	void StretchFitToChildren(bool bHor, bool bVer)
	{
		bool bWasHorFitToChildren = stretchHor == eGuiStretch::FIT_TO_CHILDREN;
		bool bWasVerFitToChildren = stretchVer == eGuiStretch::FIT_TO_CHILDREN;

		eGuiStretch hor = bHor ? eGuiStretch::FIT_TO_CHILDREN : (bWasHorFitToChildren ? eGuiStretch::NONE : stretchHor);
		eGuiStretch ver = bVer ? eGuiStretch::FIT_TO_CHILDREN : (bWasVerFitToChildren ? eGuiStretch::NONE : stretchVer);

		Stretch(hor, ver);
	}


	void StretchFillParent() { Stretch(eGuiStretch::FILL_PARENT); }
	void StretchFitToChildren() { Stretch(eGuiStretch::FIT_TO_CHILDREN); }
	void StretchNone() { Stretch(eGuiStretch::NONE); }

	void Align(eGuiAlignHor horizontalAlign, eGuiAlignVer verticalAlign) { alignHor = horizontalAlign; alignVer = verticalAlign; bLayoutNeedRefresh = true; }
	void AlignHor(eGuiAlignHor align) { alignHor = align; bLayoutNeedRefresh = true; }
	void AlignVer(eGuiAlignVer align) { alignVer = align; bLayoutNeedRefresh = true; }

	void AlignLeft() { AlignHor(eGuiAlignHor::LEFT); }
	void AlignRight() { AlignHor(eGuiAlignHor::RIGHT); }
	void AlignTop() { AlignVer(eGuiAlignVer::TOP); }
	void AlignBottom() { AlignVer(eGuiAlignVer::BOTTOM); }
	void AlignTopLeft() { Align(eGuiAlignHor::LEFT, eGuiAlignVer::TOP); }
	void AlignTopRight() { Align(eGuiAlignHor::RIGHT, eGuiAlignVer::TOP); }
	void AlignBottomLeft() { Align(eGuiAlignHor::LEFT, eGuiAlignVer::BOTTOM); }
	void AlignBottomRight() { Align(eGuiAlignHor::RIGHT, eGuiAlignVer::BOTTOM); }
	void AlignCenter() { Align(eGuiAlignHor::CENTER, eGuiAlignVer::CENTER); }
	void AlignCenterHor() { AlignHor(eGuiAlignHor::CENTER); }
	void AlignCenterVer() { AlignVer(eGuiAlignVer::CENTER); }

	void SetBgImageVisibility(bool bVisible) { bBgImageVisible = bVisible; }
	void SetBgColorVisibility(bool bVisible) { bBgColorVisible = bVisible; }

	void HideBgImage() { SetBgImageVisibility(false); }
	void HideBgColor() { SetBgColorVisibility(false); }

	void ShowBgImage() { SetBgImageVisibility(true); }
	void ShowBgColor() { SetBgColorVisibility(true); }

	void SetHoverable(bool b) { bHoverable = b; }
	void EnableHover() { SetHoverable(true); }
	void DisableHover() { SetHoverable(false); }

	template<class T>
	T* Copy(T* other);

	float GetCursorPosContentSpaceX();
	float GetCursorPosContentSpaceY();
	Vector2f GetCursorPosContentSpace();

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

	Vector2f GetPosBottomLeft() { return GetPos() + Vector2f(0, GetHeight()); }
	Vector2f GetPosBottomRight() { return GetPos() + Vector2f(GetWidth(), GetHeight()); }
	Vector2f GetPosTopLeft() { return GetPos(); }
	Vector2f GetPosTopRight() { return GetPos() + Vector2f(GetWidth(), 0); }

	const RectF& GetPadding() const { return padding; }
	const RectF& GetMargin() const { return margin; }

	float GetContentPosX() { return GetContentPos().x(); }
	float GetContentPosY() { return GetContentPos().y(); }
	Vector2f GetContentPos() { return GetContentRect().GetPos(); }
	float GetContentCenterPosY() { return GetContentPosY() + GetContentHalfHeight(); }
	float GetContentCenterPosX() { return GetContentPosX() + GetContentHalfWidth(); }
	Vector2f GetContentCenterPos() { return GetContentPos() + GetContentHalfSize(); }
	Vector2f GetContentSize() { return GetContentRect().GetSize(); }
	float GetContentSizeX() { return GetContentSize().x(); }
	float GetContentSizeY() { return GetContentSize().y(); }
	float GetContentWidth() { return GetContentRect().GetWidth(); }
	float GetContentHeight() { return GetContentRect().GetHeight(); }
	float GetContentHalfWidth() { return GetContentWidth() * 0.5f; }
	float GetContentHalfHeight() { return GetContentHeight() * 0.5f; }
	Vector2f GetContentHalfSize() { return GetContentSize() * 0.5f; }

	RectF GetRect();
	RectF GetContentRect();
	RectF GetPaddingRect();
	RectF GetBorderRect();
	RectF GetChildrenRect();

	eGuiStretch GetStretchVer() { return stretchVer; }
	eGuiStretch GetStretchHor() { return stretchHor; }

	Gui* GetParent() { return parent; }
	Gui* GetContextMenu() { return contextMenu; }
	std::vector<Gui*>& GetChildren() { return children; }

	template<class T>
	T* GetChild(int index) { return (T*)GetChildren()[index]; }

	Gui* GetChild(int index) { return GetChildren()[index]; }
	int GetIndexInParent() { return indexInParent; }

	bool IsChild(Gui* gui)
	{
		int idx = gui->GetIndexInParent();

		if (idx >= 0 && idx < GetChildren().size())
		{
			Gui* potentialChild = GetChildren()[idx];

			return potentialChild == gui;
		}

		return false;
	}


	eEventPropagationPolicy GetEventPropagationPolicy() { return eventPropagationPolicy; }

	Color& GetBgActiveColor() { return bgActiveColor; }
	Color& GetBgIdleColor() { return bgIdleColor; }
	Color& GetBgHoverColor() { return bgHoverColor; }

	Gdiplus::Bitmap* GetBgActiveImage() { return bgActiveImage; }
	Gdiplus::Bitmap* GetBgIdleImage() { return bgIdleImage; }
	Gdiplus::Bitmap* GetBgHoverImage() { return bgHoverImage; }


	bool IsPointInside(Vector2f pt) { return GetRect().IsPointInside(pt); }

	bool IsLayer() { return bLayer; }
	bool IsChildrenClipEnabled() { return bClipChildren; }
	bool IsHovered() { return bHovered; }
	bool IsCursorInside();
	bool IsHoverable() { return bHoverable; }

	const Color& GetBorderColor() { return borderColor; }
	RectF GetBorder() { return border; }
	Vector2f GetDesiredSize() { return GetSize() + Vector2f(margin.left + margin.right, margin.top + margin.bottom); }

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

	void Add(Gui* child, bool bFireEvents);
	bool Remove(Gui* child, bool bFireEvents);

	virtual Vector2f ArrangeChildren(const Vector2f& finalSize);

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

	// Is hover enabled?
	bool bHoverable;

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
	bool bLayoutNeedRefresh;

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

	// Very internal stuff, Parent's are only fillable after their size is evaluated, before it shouldn't
	bool bFillParentEnabled;
	bool bForceFitToChildren;

public:

	GuiEngine* guiEngine;

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

	Delegate<void(CursorEvent& evt)> onMouseHovering;
	Delegate<void(Gui* self, CursorEvent& evt)> onMouseHoveringClonable;

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

	Delegate<void(Gdiplus::Graphics* graphics, RectF& clipRect)> onPaint;
	Delegate<void(Gui* self, Gdiplus::Graphics* graphics, RectF& clipRect)> onPaintClonable;
};

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
	borderColor = other.borderColor;
	border = other.border;
	margin = other.margin;
	padding = other.padding;
	bLayoutNeedRefresh = other.bLayoutNeedRefresh;
	alignHor = other.alignHor;
	alignVer = other.alignVer;
	stretchHor = other.stretchHor;
	stretchVer = other.stretchVer;
	bFillParentEnabled = other.bFillParentEnabled;
	bForceFitToChildren = other.bForceFitToChildren;
	eventPropagationPolicy = other.eventPropagationPolicy;
	bHovered = other.bHovered;
	bHoverable = other.bHoverable;
	bBgColorVisible = other.bBgColorVisible;
	bBgImageVisible = other.bBgImageVisible;

	onMouseClickedClonable = other.onMouseClickedClonable;
	onMousePressedClonable = other.onMousePressedClonable;
	onMouseReleasedClonable = other.onMouseReleasedClonable;
	onMouseMovedClonable = other.onMouseMovedClonable;
	onMouseEnteredClonable = other.onMouseEnteredClonable;
	onMouseLeavedClonable = other.onMouseLeavedClonable;
	onMouseHoveringClonable = other.onMouseHoveringClonable;
	onUpdateClonable = other.onUpdateClonable;
	onTransformChangedClonable = other.onTransformChangedClonable;
	onPosChangedClonable = other.onPosChangedClonable;
	onSizeChangedClonable = other.onSizeChangedClonable;
	onParentTransformChangedClonable = other.onParentTransformChangedClonable;
	onChildTransformChangedClonable = other.onChildTransformChangedClonable;
	onParentChangedClonable = other.onParentChangedClonable;
	onChildAddedClonable = other.onChildAddedClonable;
	onChildRemovedClonable = other.onChildRemovedClonable;
	onPaintClonable = other.onPaintClonable;

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
		Add(child->Clone(), false);

	// We are root, so attach to other's parent..
	if (other.parent && other.parent->IsLayer())
		other.parent->Add(this, false);

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

inline void Gui::Add(Gui* child, bool bFireEvents)
{
	if (child->parent)
		child->parent->Remove(child, bFireEvents);

	child->parent = this;

	if (children.size() != 0)
	{
		Gui* lastControl = children[children.size() - 1];
		child->back = lastControl;
		lastControl->front = child;
	}

	child->indexInParent = children.size();
	children.push_back(child);

	if (bFireEvents)
	{
		child->onParentChanged(this);
		child->onParentChangedClonable(child, this);

		onChildAdded(child);
		onChildAddedClonable(this, child);
	}
}

inline bool Gui::Remove(Gui* child, bool bFireEvents)
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

			if (bFireEvents)
			{
				onChildRemoved(child);
				onChildRemovedClonable(this, child);

				child->onParentChanged(nullptr);
				child->onParentChangedClonable(child, nullptr);
			}

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
		return GetChild<T>(other->GetIndexInParent());
}

inline void Gui::TraverseTowardParents(const std::function<void(Gui*)>& fn)
{
	// STOP traverse 
	if (eventPropagationPolicy == eEventPropagationPolicy::STOP)
		return;

	// PROCESS fn then STOP
	if (eventPropagationPolicy == eEventPropagationPolicy::PROCESS || eventPropagationPolicy == eEventPropagationPolicy::PROCESS_STOP)
		fn(this);

	// Continue recursion
	if (eventPropagationPolicy == eEventPropagationPolicy::PROCESS || eventPropagationPolicy == eEventPropagationPolicy::AVOID)
	{
		if (back)
			back->TraverseTowardParents(fn);
		else if (parent)
			fn(parent);
	}
}

inline void Gui::Move(float dx, float dy)
{
	SetPos(pos.x() + dx, pos.y() + dy);
}

inline void Gui::SetRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty)
{
	assert(width >= 0 && height >= 0);

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
			bLayoutNeedRefresh = true;
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
	bLayoutNeedRefresh = true;
}

inline void Gui::SetPadding(float leftLength, float topLength, float rightLength, float bottomLength)
{
	padding = RectF(leftLength, topLength, rightLength, bottomLength);
	bLayoutNeedRefresh = true;
}

inline void Gui::SetBorder(float leftLength, float topLength, float rightLength, float bottomLength, const Color& color)
{
	border = RectF(leftLength, topLength, rightLength, bottomLength);
	borderColor = color;
	bLayoutNeedRefresh = true;
}

inline void Gui::RefreshLayout()
{
	Gui* arrangeRoot = this;

	while (arrangeRoot && arrangeRoot->GetParent())
		arrangeRoot = arrangeRoot->GetParent();

	arrangeRoot->Arrange(arrangeRoot->GetPos(), arrangeRoot->GetSize());
}

inline Vector2f Gui::Arrange(const Vector2f& pos, const Vector2f& size)
{
	Vector2f newPos = pos;
	Vector2f newSize = size;

	bool bFitToChildrenHor = stretchHor == eGuiStretch::FIT_TO_CHILDREN;
	bool bFitToChildrenVer = stretchVer == eGuiStretch::FIT_TO_CHILDREN;
	bool bFillParentHor = stretchHor == eGuiStretch::FILL_PARENT;
	bool bFillParentVer = stretchVer == eGuiStretch::FILL_PARENT;

	if (bForceFitToChildren)
	{
		if (bFillParentHor)
		{
			bFillParentHor = false;
			bFitToChildrenHor = true;
		}
	
		if (bFillParentVer)
		{
			bFillParentVer = false;
			bFitToChildrenVer = true;
		}

		bForceFitToChildren = false;
	}

	bool bFitToChildren = bFitToChildrenHor | bFitToChildrenVer;
	bool bFillParent = bFillParentHor || bFillParentVer;

	// Hívodjon akkor is ez ha parent FIT, ez meg FILL
	if (bFitToChildren)
	{
		// Calculate content size
		Vector2f contentSize = newSize;
		contentSize.x() -= margin.left + margin.right;
		contentSize.y() -= margin.top + margin.bottom;

		contentSize.x() -= border.left + border.right;
		contentSize.y() -= border.top + border.bottom;

		contentSize.x() -= padding.left + padding.right;
		contentSize.y() -= padding.top + padding.bottom;

		for (Gui* c : GetChildren())
			if (c->stretchHor == eGuiStretch::FILL_PARENT || c->stretchVer == eGuiStretch::FILL_PARENT)
				c->bForceFitToChildren = true;

		// Arrange children's based on our available content size
		Vector2f sizeUsed = ArrangeChildren(contentSize);

		// Convert the sizeUsed from content space to margin space
		if (bFitToChildrenHor)
		{
			sizeUsed.x() += padding.left + padding.right;
			sizeUsed.x() += border.left + border.right;
			sizeUsed.x() += margin.left + margin.right;
			newSize.x() = sizeUsed.x();
		}

		// FIT_TO_CHILDREN -> FILL_PARENT -> FILL_PARENT, a FILL_PARENT - es éppeni control - nak a minimális méretet kell felvennie mert FIT_TO_CHILDREN erõsebb náluk
		if (bFitToChildrenVer)
		{
			sizeUsed.y() += padding.top + padding.bottom;
			sizeUsed.y() += border.top + border.bottom;
			sizeUsed.y() += margin.top + margin.bottom;
			newSize.y() = sizeUsed.y();
		}

		// At this point we should enable children to fill self (eGuiStretch::FILL_PARENT)
		for (Gui* c : GetChildren())
			if(c->stretchHor == eGuiStretch::FILL_PARENT || c->stretchVer == eGuiStretch::FILL_PARENT)
				c->bFillParentEnabled = true;
	}

	if (bFillParent)
	{
		if (bFillParentHor && (parent->stretchHor != eGuiStretch::FIT_TO_CHILDREN || bFillParentEnabled))
		{
			newSize.x() = parent->GetContentSizeX();
			newPos.x() = parent->GetContentPosX();
		}
		
		if (bFillParentVer && (parent->stretchVer != eGuiStretch::FIT_TO_CHILDREN || bFillParentEnabled))
		{
			newSize.y() = parent->GetContentSizeY();
			newPos.y() = parent->GetContentPosY();
		}

		bFillParentEnabled = false;
	}

	switch (alignVer)
	{
	case eGuiAlignVer::TOP:
	{
		newPos.y() = parent->GetContentPosY();
		break;
	}
	case eGuiAlignVer::CENTER:
	{
		if (parent)
		{
			newPos.y() = parent->GetContentCenterPosY() - newSize.y() * 0.5;
		}
		break;
	}
	case eGuiAlignVer::BOTTOM:
	{
		newPos.y() = parent->GetContentRect().bottom - newSize.y();
		break;
	}
	}
	
	switch (alignHor)
	{
	case eGuiAlignHor::LEFT:
	{
		newPos.x() = parent->GetContentPosX();
		break;
	}
	case eGuiAlignHor::CENTER:
	{
		if (parent)
		{
			newPos.x() = parent->GetContentCenterPosX() - newSize.x() * 0.5;
		}
		break;
	}
	case eGuiAlignHor::RIGHT:
	{
		newPos.x() = parent->GetContentPosX() + parent->GetContentWidth() - newSize.x();
		break;
	}
	}

	// Pos and size containing the margin, subtract it
	newPos.x() += margin.left;
	newPos.y() += margin.right;
	newSize.x() -= margin.left + margin.right;
	newSize.y() -= margin.top + margin.bottom;

	SetRect(newPos.x(), newPos.y(), newSize.x(), newSize.y(), true, false);

	ArrangeChildren(newSize);

	// Here we have up to date layout, it's not dirty
	bLayoutNeedRefresh = false;

	// Arrange should return the total size used by this control so include margin in it !
	newSize.x() += margin.left + margin.right;
	newSize.y() += margin.top + margin.bottom;

	return newSize;
}

inline Vector2f Gui::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f size(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f sizeUsed = child->Arrange(child->GetPos(), child->GetDesiredSize());
		size = Vector2f::Max(size, sizeUsed);
	}
	
	return size;
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