#pragma once
#include "GuiEvent.h"
#include <BaseLibrary\Common_tmp.hpp>
#include <BaseLibrary\Platform\Window.hpp>

#include <unordered_map>

// TODO REMOVE it, seperate GDI, DX12 into seperate libs
#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b
#include <gdiplus.h>
#undef min
#undef max

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
	FILL_PARENT_POSITIVE_DIR,
	FIT_TO_CHILDREN,
};

enum class eGuiOrientation
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
class GuiMenu;
class GuiImage;
class GuiScrollable;



class Gui
{
	friend class GuiEngine;
public:
	Gui();
	Gui(GuiEngine* guiEngine);
	Gui(GuiEngine* guiEngine, bool bLayer);
	Gui(const Gui& other) { *this = other; }

	virtual ~Gui() { Clear(); }
	void Clear();

	// Important to implement in derived classes
	virtual Gui* Clone() const { return new Gui(*this); }
	Gui& operator = (const Gui& other);

	void			AddGui(Gui* child) { AddGui(child, true); }

	Gui*			AddGui();
	GuiText*		AddGuiText();
	GuiButton*		AddGuiButton();
	GuiList*		AddGuiList();
	GuiMenu*		AddGuiMenu();
	GuiSlider*		AddGuiSlider();
	GuiCollapsable* AddGuiCollapsable();
	GuiSplitter*	AddGuiSplitter();
	GuiImage*		AddGuiImage();
	GuiScrollable*	AddGuiScrollable();

	void BringToFront();

	bool RemoveGui(Gui* child) { return RemoveGui(child, true); }
	bool RemoveFromParent();

	void RefreshLayout();
	bool IsLayoutNeedRefresh() { return bLayoutNeedRefresh; }

	void Move(float dx, float dy);
	void Move(const Vector2f& delta) { Move(delta.x(), delta.y()); }

	Vector2f Arrange(const Vector2f& pos, const Vector2f& size);
	Vector2f Arrange(float posX, float posY, const Vector2f& size) { return Arrange(Vector2f(posX, posY), size); }	

	void EnableClipChildren() { SetClipChildren(true); }
	void DisableClipChildren() { SetClipChildren(false); }

	Gui*			AsPlane();
	GuiText*		AsText();
	GuiButton*		AsButton();
	GuiList*		AsList();
	GuiSlider*		AsSlider();
	GuiCollapsable*	AsCollapsable();
	GuiSplitter*	AsSplitter();
	GuiMenu*		AsMenu();

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

	//void SetEventPropagationPolicy(eEventPropagationPolicy e) { eventPropagationPolicy = e; }

	void SetClipChildren(bool b) { bClipChildren = b; }

	void SetBgToColor(const Color& idleColor, const Color& hoverColor);
	void SetBgToColor(const Color& color) { SetBgToColor(color, color);}
	void SetBgIdleColor(const Color& color);
	void SetBgHoverColor(const Color& color);
	void SetBgActiveColor(const Color& color) { bgActiveColor = color; }
	void SetBgActiveColorToIdle();
	void SetBgActiveColorToHover();

	void SetBgToImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath, int width = 0, int height = 0);
	void SetBgIdleImage(const std::wstring& path, int width = 0, int height = 0);
	void SetBgHoverImage(const std::wstring& path, int width = 0, int height = 0);
	void SetBgActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }
	void SetBgActiveImageToIdle();
	void SetBgActiveImageToHover();

	void SetBorder(float leftLength, float rightLength, float topLength, float bottomLength, const Color& color);
	void SetBorder(float borderLength, const Color& color) { SetBorder(borderLength, borderLength, borderLength, borderLength, color); }

	void SetMargin(float leftLength, float rightLength, float topLength, float bottomLength);
	void SetMargin(float length) { SetMargin(length, length, length, length); }

	void SetMarginLeft(float length)	{ SetMargin(length, margin.right, margin.top, margin.bottom); }
	void SetMarginRight(float length)	{ SetMargin(margin.left, length, margin.top, margin.bottom); }
	void SetMarginTop(float length)		{ SetMargin(margin.left, margin.right, length, margin.bottom); }
	void SetMarginBottom(float length)	{ SetMargin(margin.left, margin.right, margin.top, length); }

	void SetPadding(float leftLength, float rightLength, float topLength, float bottomLength);
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

	void StretchFillParent(bool bHor, bool bVer);
	void StretchFitToChildren(bool bHor, bool bVer);


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
	void AlignHorCenter() { AlignHor(eGuiAlignHor::CENTER); }
	void AlignVerCenter() { AlignVer(eGuiAlignVer::CENTER); }

	void SetBgImageVisible(bool bVisible) { bBgImageVisible = bVisible; }
	void SetBgColorVisible(bool bVisible) { bBgColorVisible = bVisible; }

	void HideBgImage() { SetBgImageVisible(false); }
	void HideBgColor() { SetBgColorVisible(false); }

	void ShowBgImage() { SetBgImageVisible(true); }
	void ShowBgColor() { SetBgColorVisible(true); }

	void SetHoverable(bool b) { bHoverable = b; }
	void EnableHover() { SetHoverable(true); for (auto& c : GetChildren())c->SetHoverable(true); }
	void DisableHover() { SetHoverable(false); for (auto& c : GetChildren())c->SetHoverable(false);	}
	void DisableChildrenHover() { for (auto& child : GetChildren()) child->DisableHover(); }

	void SetBgFreeze(bool b) { bBgFreezed = b; }
	void FreezeBg() { SetBgFreeze(true); }
	void UnfreezeBg() { SetBgFreeze(false); }

	void SetBgStateToIdle() { bgActiveImage = bgIdleImage; bgActiveColor = bgIdleColor; }
	void SetBgStateToHover() { bgActiveImage = bgHoverImage; bgActiveColor = bgHoverColor; }

	float GetCursorPosContentSpaceX();
	float GetCursorPosContentSpaceY();
	Vector2f GetCursorPosContentSpace();

	float GetPosRight()	 { return GetPosX() + GetWidth(); }
	float GetPosLeft()	 { return GetPosX(); }
	float GetPosTop()	 { return GetPosY(); }
	float GetPosBottom() { return GetPosY() + GetHeight(); }

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

	// TODO SetMinSize SetMaxSize
	Vector2f GetMinSize() { return Vector2f(1, 1); }
	float GetMinSizeX() { return GetMinSize().x(); }
	float GetMinSizeY() { return GetMinSize().y(); }

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

	float GetContentRight() { return GetContentRect().right; }
	float GetContentLeft() { return GetContentRect().left; }
	float GetContentBottom() { return GetContentRect().bottom; }
	float GetContentTop() { return GetContentRect().top; }

	RectF GetRect();
	RectF GetContentRect();
	RectF GetPaddingRect();
	RectF GetBorderRect();
	RectF GetVisibleRect();
	RectF GetVisibleContentRect();
	RectF GetVisiblePaddingRect();
	RectF GetChildrenRect();

	eGuiStretch GetStretchVer() { return stretchVer; }
	eGuiStretch GetStretchHor() { return stretchHor; }

	Gui* GetParent() { return parent; }
	Gui* GetContextMenu() { return contextMenu; }
	std::vector<Gui*>& GetChildren() { return children; }

	template<class T>
	std::vector<T*> GetChildrenRecursive();

	template<class T>
	T* GetChild(int index) { assert(dynamic_cast<T*>(GetChildren()[index])); return (T*)GetChildren()[index]; }

	Gui* GetChild(int index) { assert(index >= 0); return (index >= GetChildren().size()) ? nullptr : GetChildren()[index]; }
	int GetIndexInParent() { return indexInParent; }

	//eEventPropagationPolicy GetEventPropagationPolicy() { return eventPropagationPolicy; }

	const Color& GetBgActiveColor() const { return bgActiveColor; }
	const Color& GetBgIdleColor() const { return bgIdleColor; }
	const Color& GetBgHoverColor() const { return bgHoverColor; }

	Gdiplus::Bitmap* GetBgActiveImage() { return bgActiveImage; }
	Gdiplus::Bitmap* GetBgIdleImage() { return bgIdleImage; }
	Gdiplus::Bitmap* GetBgHoverImage() { return bgHoverImage; }

	const Color& GetBorderColor() { return borderColor; }
	RectF GetBorder() { return border; }
	Vector2f GetDesiredSize() { return GetSize() + Vector2f(margin.left + margin.right, margin.top + margin.bottom); }

	const std::wstring& GetName() { return name; }

	template<class T>
	bool Is();

	bool IsPointInside(Vector2f pt) { return GetRect().IsPointInside(pt); }

	bool IsLayer() { return bLayer; }
	bool IsChildrenClipEnabled() { return bClipChildren; }
	bool IsHovered() { return bHovered; }
	bool IsBgFreezed() { return bBgFreezed; }
	bool IsCursorInside();
	bool IsHoverable() { return bHoverable; }

	bool IsChild(Gui* gui);
	bool IsSibling(Gui* child);

protected:
	void SetVisibleRect(const RectF& rect) { visibleRect = rect; }

	void SetContentRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty);
	void SetContentRect(const RectF rect, bool bMoveChildren, bool bMakeLayoutDirty) { SetContentRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bMoveChildren, bMakeLayoutDirty); }

	void SetRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty);
	void SetRect(const RectF& rect, bool bMoveChildren, bool bMakeLayoutDirty) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bMoveChildren, bMakeLayoutDirty); }
	
	void SetContentSize(float width, float height, bool bMakeLayoutDirty) { SetContentRect(GetContentRect().left, GetContentRect().top, width, height, false, bMakeLayoutDirty); }
	void SetContentSize(const Vector2f& s, bool bMakeLayoutDirty) { SetContentSize(s.x(), s.y(), bMakeLayoutDirty); }

	void SetSize(const Vector2f& s, bool bMakeLayoutDirty) { SetSize(s.x(), s.y(), bMakeLayoutDirty); }
	void SetSize(float width, float height, bool bMakeLayoutDirty) { SetRect(pos.x(), pos.y(), width, height, false, bMakeLayoutDirty); }

	void SetActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }

	void AddGui(Gui* child, bool bFireEvents);
	bool RemoveGui(Gui* child, bool bFireEvents);

	template<class T>
	T* Copy(T* other);

	void TraverseTowardParents(const std::function<void(Gui*)>& fn);

	virtual Vector2f ArrangeChildren(const Vector2f& finalSize);

protected:
	// Name it however you want
	std::wstring name;

	// Position
	Vector2f pos;

	// Width, Height
	Vector2f size;

	// Children widgets
	std::vector<Gui*> children;

	// Parent widget
	Gui* parent;

	// Alignment & Stretch
	eGuiAlignHor alignHor;
	eGuiAlignVer alignVer;
	eGuiStretch	 stretchHor;
	eGuiStretch	 stretchVer;

	// Border
	Color borderColor;
	RectF border;

	// Margin
	RectF margin;

	// Padding
	RectF padding;

	// IsLayer ?
	bool bLayer;

	// Is clipped by parent content area?
	bool bClipChildren;

	// Is hovered by cursor?
	bool bHovered;

	// Is hover enabled?
	bool bHoverable;

	bool bBgFreezed;

	// children index in parent
	int indexInParent;

	// What this widget should do with the message it reaches while moving up in hierarchy (toward parents)
	//eEventPropagationPolicy eventPropagationPolicy;

	// Background image and color
	bool bBgImageVisible;
	bool bBgColorVisible;
	Gdiplus::Bitmap* bgIdleImage;
	Gdiplus::Bitmap* bgHoverImage;
	Gdiplus::Bitmap* bgActiveImage;

	Color bgIdleColor;
	Color bgHoverColor;
	Color bgActiveColor;

	// If true, RefreshLayout() will be called before render, to ReArrange the layout as necessary
	bool bLayoutNeedRefresh;

	RectF visibleRect;

	// Layering (Our neighbours in the tree hierarchy)
	Gui* front;
	Gui* back;

	// Attached context menu
	Gui* contextMenu;

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

	Delegate<void(DragData& data)> onOperSysDragEntered;
	Delegate<void(DragData& data)> onOperSysDragLeaved;
	Delegate<void(DragData& data)> onOperSysDragHovering;
	Delegate<void(DragData& data)> onOperSysDropped;

	Delegate<void(float deltaTime)> onUpdate;
	Delegate<void(Gui* self, float deltaTime)> onUpdateClonable;

	Delegate<void(RectF& rect)> onTransformChanged;
	Delegate<void(Gui* self, RectF& rect)> onTransformChangedClonable;

	Delegate<void(Vector2f pos)> onPosChanged;
	Delegate<void(Gui* self, Vector2f pos)> onPosChangedClonable;

	Delegate<void(Vector2f size)> onSizeChanged;
	Delegate<void(Gui* self, Vector2f size)> onSizeChangedClonable;

	Delegate<void(RectF rect)> onRectChanged;
	Delegate<void(Gui* self, RectF rect)> onRectChangedClonable;

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

	Delegate<void(Gdiplus::Graphics* graphics)> onPaint;
	Delegate<void(Gui* self, Gdiplus::Graphics* graphics)> onPaintClonable;
};

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

template<class T>
std::vector<T*> Gui::GetChildrenRecursive()
{
	std::vector<T*> result;

	std::function<void(Gui* gui, std::vector<T*>& result)> traverse;
	traverse = [&traverse](Gui* gui, std::vector<T*>& result)
	{
		if (gui->Is<T>())
			result.push_back((T*)gui);

		for (Gui* a : gui->GetChildren())
			traverse(a, result);
	};

	traverse(this, result);

	return result;
}

template<class T>
bool Gui::Is()
{
	return dynamic_cast<T*>(this) != nullptr;
}

} // namespace inl::gui