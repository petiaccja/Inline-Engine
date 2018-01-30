#pragma once
#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Color.hpp>
#include <unordered_map>

#include "Event.hpp"
#include "Rect.hpp"

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
	FILL_SPACE,
	FILL_SPACE_POSITIVE_DIR,
	FIT_TO_CONTENT,
};

enum class eGuiOrientation
{
	VERTICAL,
	HORIZONTAL,
};

class GuiEngine;
class Gui;
class Text;
class Button;
class ListView;
class Slider;
class Collapsable;
class Splitter;
class Menu;
class Image;
class ScrollableArea;



class Gui
{
	friend class GuiEngine;
public:
	//Gui();
	Gui(GuiEngine* guiEngine);
	Gui(GuiEngine* guiEngine, bool bLayer);
	Gui(const Gui& other):guiEngine(other.guiEngine) { *this = other; }

	virtual ~Gui() { Clear(); }
	void Clear();

	// Important to implement in derived classes
	virtual Gui* Clone() const { return new Gui(*this); }
	Gui& operator = (const Gui& other);

	Gui* AddGui() { return AddGui<Gui>(); }
	void AddGui(Gui* child) { AddGui(child, true); }

	template<class T>
	T* AddGui()
	{
		T* p = new T(guiEngine);
		AddGui(p);
		return p;
	}

	void BringToFront();

	bool RemoveGui(Gui* child) { return RemoveGui(child, true); }
	bool RemoveFromParent();

	void RefreshLayout();
	bool IsLayoutNeedRefresh() { return bLayoutNeedRefresh; }

	void Move(float dx, float dy);
	void Move(const Vec2& delta) { Move(delta.x, delta.y); }

	Vec2 Arrange(const Vec2& pos, const Vec2& size);
	virtual Vec2 ArrangeChildren();

	void EnableClipChildren() { SetClipChildren(true); }
	void DisableClipChildren() { SetClipChildren(false); }

	template<class T>
	T& As() { assert(Is<T>());  return *(T*)this; }

	void SetContentRect(float x, float y, float width, float height) { SetContentRect(x, y, width, height, true, true); }
	void SetContentRect(const GuiRectF rect) { SetContentRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()); }

	void SetRect(float x, float y, float width, float height) { SetRect(x, y, width, height, true, true); }
	void SetRect(const GuiRectF& rect) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()); }

	void SetName(const std::wstring& str) { name = str; }
	void SetName(const std::string& str) { SetName(std::wstring(str.begin(), str.end())); }

	void SetContextMenu(Gui* c) { contextMenu = c; }
	void SetPos(const Vec2& p) { SetPos(p.x, p.y); }
	void SetPos(float x, float y) { SetRect(x, y, size.x, size.y); }
	void SetCenterPos(float x, float y) { SetPos(x - GetHalfWidth(), y + GetHalfHeight()); }
	void SetPosX(float x) { SetRect(x, pos.y, size.x, size.y); }
	void SetPosY(float y) { SetRect(pos.x, y, size.x, size.y); }

	void SetWidth(float w) { SetSize(Vec2(w, size.y)); }
	void SetHeight(float h) { SetSize(Vec2(size.x, h)); }

	void SetSize(const Vec2& s) { SetSize(s.x, s.y); }
	void SetSize(float width, float height) { SetRect(pos.x, pos.y, width, height, true, true); }

	void SetContentSize(float width, float height) { SetContentRect(GetContentRect().left, GetContentRect().top, width, height, true, true); }
	void SetContentSize(const Vec2& s) { SetContentSize(s.x, s.y); }

	//void SetEventPropagationPolicy(eEventPropagationPolicy e) { eventPropagationPolicy = e; }

	void SetClipChildren(bool b) { bClipChildren = b; }

	void SetBgToColor(const ColorI& idleColor, const ColorI& hoverColor);
	void SetBgToColor(const ColorI& color) { SetBgToColor(color, color);}
	void SetBgIdleColor(const ColorI& color);
	void SetBgHoverColor(const ColorI& color);
	void SetBgActiveColor(const ColorI& color) { bgActiveColor = color; }
	void SetBgActiveColorToIdle();
	void SetBgActiveColorToHover();

	void SetBgToImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath, int width = 0, int height = 0);
	void SetBgIdleImage(const std::wstring& path, int width = 0, int height = 0);
	void SetBgHoverImage(const std::wstring& path, int width = 0, int height = 0);
	void SetBgActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }
	void SetBgActiveImageToIdle();
	void SetBgActiveImageToHover();

	void SetBorder(float leftLength, float rightLength, float topLength, float bottomLength, const ColorI& color);
	void SetBorder(float borderLength, const ColorI& color) { SetBorder(borderLength, borderLength, borderLength, borderLength, color); }

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

	void StretchHorFillParent() {StretchHor(eGuiStretch::FILL_SPACE); }
	void StretchHorFitToContent() { StretchHor(eGuiStretch::FIT_TO_CONTENT); }
	void StretchHorNone() { StretchHor(eGuiStretch::NONE); }

	void StretchVerFillParent() { StretchVer(eGuiStretch::FILL_SPACE); }
	void StretchVerFitToContent() { StretchVer(eGuiStretch::FIT_TO_CONTENT); }
	void StretchVerNone() { StretchVer(eGuiStretch::NONE); }

	void StretchFillParent(bool bHor, bool bVer);
	void StretchFitToContent(bool bHor, bool bVer);


	void StretchFillParent() { Stretch(eGuiStretch::FILL_SPACE); }
	void StretchFitToContent() { Stretch(eGuiStretch::FIT_TO_CONTENT); }
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
	Vec2 GetCursorPosContentSpace();

	float GetPosX() { return pos.x; }
	float GetPosY() { return pos.y; }
	const Vec2& GetPos() { return pos; }
	float GetCenterPosX() { return pos.x + GetHalfWidth(); }
	float GetCenterPosY() { return pos.y + GetHalfHeight(); }
	Vec2 GetCenterPos() { return pos + GetHalfSize(); }
	const Vec2& GetSize() { return size; }
	float GetSizeX() { return size.x; }
	float GetSizeY() { return size.y; }
	float GetWidth() { return size.x; }
	float GetHeight() { return size.y; }
	float GetHalfWidth() { return GetWidth() * 0.5f; }
	float GetHalfHeight() { return GetHeight() * 0.5f; }
	Vec2 GetHalfSize() { return Vec2(GetHalfWidth(), GetHalfHeight()); }

	// TODO SetMinSize SetMaxSize
	Vec2 GetMinSize() { return Vec2(1, 1); }
	//float GetMinSizeX() { return GetMinSize().x; }
	//float GetMinSizeY() { return GetMinSize().y; }

	Vec2 GetPosBottomLeft() { return GetPos() + Vec2(0, GetHeight()); }
	Vec2 GetPosBottomRight() { return GetPos() + Vec2(GetWidth(), GetHeight()); }
	Vec2 GetPosTopLeft() { return GetPos(); }
	Vec2 GetPosTopRight() { return GetPos() + Vec2(GetWidth(), 0); }

	const GuiRectF& GetPadding() const { return padding; }
	const GuiRectF& GetMargin() const { return margin; }

	//float GetContentPosX() { return GetContentPos().x; }
	//float GetContentPosY() { return GetContentPos().y; }
	Vec2 GetContentPos() { return GetContentRect().GetTopLeft(); }
	//float GetContentCenterPosY() { return GetContentPos().y + GetContentSize().y * 0.5; }
	//float GetContentCenterPosX() { return GetContentPos().x + GetContentSize().x * 0.5; }
	Vec2 GetContentCenterPos() { return GetContentPos() + GetContentSize() * 0.5; }
	Vec2 GetContentSize() { return GetContentRect().GetSize(); }
	//float GetContentSizeX() { return GetContentSize().x; }
	//float GetContentSizeY() { return GetContentSize().y; }
	//float GetContentWidth() { return GetContentRect().GetWidth(); }
	//float GetContentHeight() { return GetContentRect().GetHeight(); }
	//float GetContentHalfWidth() { return GetContentWidth() * 0.5f; }
	//float GetContentHalfHeight() { return GetContentHeight() * 0.5f; }
	//Vec2 GetContentHalfSize() { return GetContentSize() * 0.5f; }

	//float GetContentRight() { return GetContentRect().right; }
	//float GetContentLeft() { return GetContentRect().left; }
	//float GetContentBottom() { return GetContentRect().bottom; }
	//float GetContentTop() { return GetContentRect().top; }

	GuiRectF GetRect();
	GuiRectF GetContentRect();
	GuiRectF GetPaddingRect();
	GuiRectF GetBorderRect();
	GuiRectF GetVisibleRect();
	GuiRectF GetVisibleContentRect();
	GuiRectF GetVisiblePaddingRect();
	GuiRectF GetChildrenRect();

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

	const ColorI& GetBgActiveColor() const { return bgActiveColor; }
	const ColorI& GetBgIdleColor() const { return bgIdleColor; }
	const ColorI& GetBgHoverColor() const { return bgHoverColor; }

	Gdiplus::Bitmap* GetBgActiveImage() { return bgActiveImage; }
	Gdiplus::Bitmap* GetBgIdleImage() { return bgIdleImage; }
	Gdiplus::Bitmap* GetBgHoverImage() { return bgHoverImage; }

	const ColorI& GetBorderColor() { return borderColor; }
	GuiRectF GetBorder() { return border; }
	Vec2 GetDesiredSize() { return GetSize() + Vec2(margin.left + margin.right, margin.top + margin.bottom); }

	const std::wstring& GetName() { return name; }

	template<class T>
	bool Is();

	bool IsPointInside(Vec2 pt) { return GetRect().IsPointInside(pt); }

	bool IsLayer() { return bLayer; }
	bool IsChildrenClipEnabled() { return bClipChildren; }
	bool IsHovered() { return bHovered; }
	bool IsBgFreezed() { return bBgFreezed; }
	bool IsCursorInside();
	bool IsHoverable() { return bHoverable; }

	bool IsChild(Gui& gui);
	bool IsSibling(Gui& child);

protected:
	void SetVisibleRect(const GuiRectF& rect) { visibleRect = rect; }

	void SetContentRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty);
	void SetContentRect(const GuiRectF rect, bool bMoveChildren, bool bMakeLayoutDirty) { SetContentRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bMoveChildren, bMakeLayoutDirty); }

	void SetRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty);
	void SetRect(const GuiRectF& rect, bool bMoveChildren, bool bMakeLayoutDirty) { SetRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), bMoveChildren, bMakeLayoutDirty); }
	
	void SetContentSize(float width, float height, bool bMakeLayoutDirty) { SetContentRect(GetContentRect().left, GetContentRect().top, width, height, false, bMakeLayoutDirty); }
	void SetContentSize(const Vec2& s, bool bMakeLayoutDirty) { SetContentSize(s.x, s.y, bMakeLayoutDirty); }

	void SetSize(const Vec2& s, bool bMakeLayoutDirty) { SetSize(s.x, s.y, bMakeLayoutDirty); }
	void SetSize(float width, float height, bool bMakeLayoutDirty) { SetRect(pos.x, pos.y, width, height, false, bMakeLayoutDirty); }

	void SetActiveImage(Gdiplus::Bitmap* image) { bgActiveImage = image; }

	void AddGui(Gui* child, bool bFireEvents);
	bool RemoveGui(Gui* child, bool bFireEvents);

	template<class T>
	T* Copy(T* other);

	void TraverseTowardParents(GuiEvent& event, const std::function<void(Gui&)>& eventHandler);

protected:
	// Name it however you want
	std::wstring name;

	// Position
	Vec2 pos;

	// Width, Height
	Vec2 size;

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
	ColorI borderColor;
	GuiRectF border;

	// Margin
	GuiRectF margin;

	// Padding
	GuiRectF padding;

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

	ColorI bgIdleColor;
	ColorI bgHoverColor;
	ColorI bgActiveColor;

	// If true, RefreshLayout() will be called before render, to ReArrange the layout as necessary
	bool bLayoutNeedRefresh;

	GuiRectF visibleRect;

	// Layering (Our neighbours in the tree hierarchy)
	Gui* front;
	Gui* back;

	// Attached context menu
	Gui* contextMenu;

	// Very internal stuff, Parent's are only fillable after their size is evaluated, before it shouldn't
	bool bFillParentEnabled;
	bool bForceFitToContent;
public:

	GuiEngine* guiEngine;

	// Public events
	Event<CursorEvent&> OnCursorClick;
	Event<CursorEvent&> OnCursorDblClick;
	Event<CursorEvent&> OnCursorPress;
	Event<CursorEvent&> OnCursorRelease;
	Event<CursorEvent&> OnCursorMove;
	Event<CursorEvent&> OnCursorEnter;
	Event<CursorEvent&> OnCursorLeave;
	Event<CursorEvent&> OnCursorHover;
	Event<DragDropEvent&> OnOperSysDragEnter;
	Event<DragDropEvent&> OnOperSysDragLeave;
	Event<DragDropEvent&> OnOperSysDragHover;
	Event<DragDropEvent&> OnOperSysDrop;
	Event<UpdateEvent&> OnUpdate;
	Event<TransformEvent&> OnTransformChange;
	Event<TransformEvent&> OnParentTransformChange;
	Event<TransformEvent&> OnChildTransformChange;
	Event<ParentEvent&> OnParentChange;
	Event<ChildEvent&> OnChildAdd;
	Event<ChildEvent&> OnChildRemove;
	Event<PaintEvent&> OnPaint;
};

template<class T>
T* Gui::Copy(T* other)
{
	assert(other);

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