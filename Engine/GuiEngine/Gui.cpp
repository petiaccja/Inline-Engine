#pragma once
#include "Gui.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSplitter.hpp"
#include "GuiMenu.h"
#include "GuiEngine.hpp"

using namespace inl::gui;

Gui::Gui(GuiEngine* guiEngine)
:Gui(guiEngine, false)
{
	guiEngine->Register(this);
}

Gui::Gui()
:Gui(nullptr, false)
{
	guiEngine->Register(this);
}

Gui::Gui(GuiEngine* guiEngine, bool bLayer)
{
	borderColor = Color(128);
	size = Vector2f(60, 20);
	bgIdleColor = Color(45);
	bgHoverColor = Color(75);
	border = RectF(0, 0, 0, 0);
	pos = Vector2f(0, 0);
	eventPropagationPolicy = eEventPropagationPolicy::PROCESS;
	indexInParent = -1;
	this->bLayer = bLayer;
	parent = nullptr;
	front = nullptr;
	back = nullptr;
	contextMenu = nullptr;
	bgActiveImage = nullptr;
	bgIdleImage = nullptr;
	bgHoverImage = nullptr;
	bBgImageVisible = true;
	bBgColorVisible = true;
	bClipChildren = true;
	margin = RectF(0, 0, 0, 0);
	padding = RectF(0, 0, 0, 0);
	bLayoutNeedRefresh = false;
	alignHor = eGuiAlignHor::NONE;
	alignVer = eGuiAlignVer::NONE;
	stretchHor = eGuiStretch::NONE;
	stretchVer = eGuiStretch::NONE;
	bHovered = false;
	bHoverable = true;
	bFillParentEnabled = false;
	bForceFitToChildren = false;
	this->guiEngine = guiEngine;
	privateData = nullptr;

	SetBgActiveColor(bgIdleColor);

	onMouseEnteredClonable += [](Gui* self, CursorEvent& event)
	{
		if (!self->guiEngine->IsHoverFreezed())
		{
			self->SetBgActiveColor(self->GetBgHoverColor());
			self->SetBgActiveImage(self->GetBgHoverImage());
			self->bHovered = true;
		}
	};

	onMouseLeavedClonable += [](Gui* self, CursorEvent& event)
	{
		if (!self->guiEngine->IsHoverFreezed())
		{
			self->SetBgActiveColor(self->GetBgIdleColor());
			self->SetBgActiveImage(self->GetBgIdleImage());
			self->bHovered = false;
		}
	};

	onChildRemovedClonable += [](Gui* self, Gui* child)
	{
		self->bLayoutNeedRefresh = true;
	};

	onChildAddedClonable += [](Gui* self, Gui* child)
	{
		self->bLayoutNeedRefresh = true;
	};

	onPaintClonable += [](Gui* self, Gdiplus::Graphics* graphics)
	{
		if (self->IsLayoutNeedRefresh())
			self->RefreshLayout();

		RectF paddingRect = self->GetPaddingRect();
		RectF borderRect = self->GetBorderRect();

		Gdiplus::Rect gdiBorderRect(borderRect.left, borderRect.top, borderRect.GetWidth(), borderRect.GetHeight());
		Gdiplus::Rect gdiPaddingRect(paddingRect.left, paddingRect.top, paddingRect.GetWidth(), paddingRect.GetHeight());

		// TODO visibleRect
		auto visibleContentRect = self->GetVisibleRect();
		Gdiplus::Rect gdiClipRect(visibleContentRect.left, visibleContentRect.top, visibleContentRect.GetWidth(), visibleContentRect.GetHeight());

		// Clipping
		graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeReplace);

		// Draw left border
		Color borderColor = self->GetBorderColor();
		RectF border = self->GetBorder();

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
		if (self->GetBgActiveImage() && self->bBgImageVisible)
		{
			graphics->DrawImage(self->GetBgActiveImage(), gdiPaddingRect);
		}
		else if (self->bBgColorVisible) // Draw Background Colored Rectangle
		{
			Color bgColor = self->GetBgActiveColor();
			Gdiplus::SolidBrush brush(Gdiplus::Color(bgColor.a, bgColor.r, bgColor.g, bgColor.b));
			graphics->FillRectangle(&brush, gdiPaddingRect);
		}
	};
}

void Gui::Clear()
{
	front = nullptr;
	back = nullptr;
	parent = nullptr;
	indexInParent = -1;

	children.clear();
}

Gui& Gui::operator = (const Gui& other)
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

void Gui::InitFromImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath)
{
	SetBgToImage(idleImagePath, hoverImagePath);
	SetSize(Vector2f(GetBgIdleImage()->GetWidth(), GetBgIdleImage()->GetHeight()));
}

void Gui::Add(Gui* child, bool bFireEvents)
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

bool Gui::Remove(Gui* child, bool bFireEvents)
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

bool Gui::RemoveFromParent()
{
	if (parent)
		return parent->Remove(this);

	return false;
}

void Gui::TraverseTowardParents(const std::function<void(Gui*)>& fn)
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

void Gui::Move(float dx, float dy)
{
	SetPos(pos.x() + dx, pos.y() + dy);
}

void Gui::SetRect(float x, float y, float width, float height, bool bMoveChildren, bool bMakeLayoutDirty)
{
	// In debug tell us if there's a problem with sizing
	//assert(width >= 0 && height >= 0);
	width = std::max(width, GetMinSizeX()); // In release solve the issue
	height = std::max(height, GetMinSizeY());

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
			if (bMoveChildren)
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
		onSizeChangedClonable(this, rect.GetSize());

		if (bMakeLayoutDirty)
			bLayoutNeedRefresh = true;
	}
}

void Gui::SetContentRect(float x, float y, float width, float height, bool bFireEvents, bool bMoveChildren)
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

void Gui::SetBgIdleImage(const std::wstring& str)
{
	Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(str.c_str());

	if (bgIdleImage == GetBgActiveImage())
		SetBgActiveImage(newBitmap);

	if (bgIdleImage)
		delete bgIdleImage;

	bgIdleImage = newBitmap;
}

void Gui::SetBgHoverImage(const std::wstring& str)
{
	Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(str.c_str());

	if (bgHoverImage == GetBgActiveImage())
		SetBgActiveImage(newBitmap);

	if (bgHoverImage)
		delete bgHoverImage;

	bgHoverImage = newBitmap;
}

void Gui::SetBgIdleColor(const Color& color)
{
	if (bgIdleColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgIdleColor = color;
}

void Gui::SetBgToColor(const Color& idleColor, const Color& hoverColor)
{
	// Disable Image !
	HideBgImage();

	SetBgHoverColor(hoverColor);
	SetBgIdleColor(idleColor);
}

void Gui::SetBgHoverColor(const Color& color)
{
	if (bgHoverColor == GetBgActiveColor())
		SetBgActiveColor(color);

	bgHoverColor = color;
}

void Gui::SetBgActiveColorToIdle()
{
	SetBgActiveColor(GetBgIdleColor());
}

void Gui::SetBgActiveColorToHover()
{
	SetBgActiveColor(GetBgHoverColor());
}

void Gui::SetBgColorForAllStates(const Color& color)
{
	SetBgHoverColor(color);
	SetBgIdleColor(color);
}

void Gui::SetBgToImage(const std::wstring& idleImagePath, const std::wstring& hoverImagePath)
{
	HideBgColor();

	SetBgIdleImage(idleImagePath);
	SetBgHoverImage(hoverImagePath);
}

void Gui::SetBgActiveImageToIdle()
{
	SetBgActiveImage(bgIdleImage);
}

void Gui::SetBgActiveImageToHover()
{
	SetBgActiveImage(bgHoverImage);
}

void Gui::SetBgImageForAllStates(const std::wstring& filePath)
{
	SetBgIdleImage(filePath);
	SetBgHoverImage(filePath);
}

void Gui::SetMargin(float leftLength, float topLength, float rightLength, float bottomLength)
{
	margin = RectF(leftLength, topLength, rightLength, bottomLength);
	bLayoutNeedRefresh = true;
}

void Gui::SetPadding(float leftLength, float topLength, float rightLength, float bottomLength)
{
	padding = RectF(leftLength, topLength, rightLength, bottomLength);
	bLayoutNeedRefresh = true;
}
void Gui::SetBorder(float leftLength, float topLength, float rightLength, float bottomLength, const Color& color)
{
	border = RectF(leftLength, topLength, rightLength, bottomLength);
	borderColor = color;
	bLayoutNeedRefresh = true;
}

void Gui::StretchFillParent(bool bHor, bool bVer)
{
	bool bWasHorFill = stretchHor == eGuiStretch::FILL_PARENT;
	bool bWasVerFill = stretchVer == eGuiStretch::FILL_PARENT;

	eGuiStretch hor = bHor ? eGuiStretch::FILL_PARENT : (bWasHorFill ? eGuiStretch::NONE : stretchHor);
	eGuiStretch ver = bVer ? eGuiStretch::FILL_PARENT : (bWasVerFill ? eGuiStretch::NONE : stretchVer);

	Stretch(hor, ver);
}

void Gui::StretchFitToChildren(bool bHor, bool bVer)
{
	bool bWasHorFitToChildren = stretchHor == eGuiStretch::FIT_TO_CHILDREN;
	bool bWasVerFitToChildren = stretchVer == eGuiStretch::FIT_TO_CHILDREN;

	eGuiStretch hor = bHor ? eGuiStretch::FIT_TO_CHILDREN : (bWasHorFitToChildren ? eGuiStretch::NONE : stretchHor);
	eGuiStretch ver = bVer ? eGuiStretch::FIT_TO_CHILDREN : (bWasVerFitToChildren ? eGuiStretch::NONE : stretchVer);

	Stretch(hor, ver);
}

void Gui::RefreshLayout()
{
	Gui* arrangeRoot = this;

	while (arrangeRoot && arrangeRoot->GetParent())
		arrangeRoot = arrangeRoot->GetParent();

	arrangeRoot->Arrange(arrangeRoot->GetPos(), arrangeRoot->GetSize());
}

Vector2f Gui::Arrange(const Vector2f& pos, const Vector2f& size)
{
	Vector2f newPos = pos;
	Vector2f newSize = size;

	bool bFitToChildrenHor = stretchHor == eGuiStretch::FIT_TO_CHILDREN;
	bool bFitToChildrenVer = stretchVer == eGuiStretch::FIT_TO_CHILDREN;
	bool bFillParentHor = stretchHor == eGuiStretch::FILL_PARENT;
	bool bFillParentVer = stretchVer == eGuiStretch::FILL_PARENT;
	bool bFillParentPositibeDirHor = stretchHor == eGuiStretch::FILL_PARENT_POSITIVE_DIR;
	bool bFillParentPositibeDirVer = stretchVer == eGuiStretch::FILL_PARENT_POSITIVE_DIR;

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
	bool bFillParentPositiveDir = bFillParentPositibeDirHor || bFillParentPositibeDirVer;

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

		// FIT_TO_CHILDREN -> FILL_PARENT -> FILL_PARENT, a FILL_PARENT,  parent -> children -> children.... parent will be dominant against "FILL_PARENT" flagged children, so everybody should take the minimal size
		if (bFitToChildrenVer)
		{
			sizeUsed.y() += padding.top + padding.bottom;
			sizeUsed.y() += border.top + border.bottom;
			sizeUsed.y() += margin.top + margin.bottom;
			newSize.y() = sizeUsed.y();
		}

		// At this point we should enable children to fill self (eGuiStretch::FILL_PARENT)
		for (Gui* c : GetChildren())
			if (c->stretchHor == eGuiStretch::FILL_PARENT || c->stretchVer == eGuiStretch::FILL_PARENT ||
				c->stretchHor == eGuiStretch::FILL_PARENT_POSITIVE_DIR || c->stretchVer == eGuiStretch::FILL_PARENT_POSITIVE_DIR)
			{
				c->bFillParentEnabled = true;
			}
	}

	if (bFillParent || bFillParentPositiveDir)
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

		if (bFillParentPositibeDirHor && (parent->stretchHor != eGuiStretch::FIT_TO_CHILDREN || bFillParentEnabled))
		{
			newSize.x() = parent->GetContentRight() - newPos.x();
		}

		if (bFillParentPositibeDirVer && (parent->stretchVer != eGuiStretch::FIT_TO_CHILDREN || bFillParentEnabled))
		{
			newSize.y() = parent->GetContentBottom() - newPos.y();
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

Vector2f Gui::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f size(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f sizeUsed = child->Arrange(child->GetPos(), child->GetDesiredSize());
		size = Vector2f::Max(size, sizeUsed);
	}

	return size;
}

RectF Gui::GetRect()
{
	return RectF(pos, size);
}

RectF Gui::GetVisibleRect()
{
	return visibleRect;
}

RectF Gui::GetVisibleContentRect()
{
	RectF rect = GetVisibleRect();

	rect.Intersect(GetContentRect());

	return rect;
}

RectF Gui::GetVisiblePaddingRect()
{
	RectF rect = GetVisibleRect();

	rect.Intersect(GetPaddingRect());

	return rect;
}

RectF Gui::GetContentRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-border);
	result.MoveSidesLocal(-padding);

	return result;
}

RectF Gui::GetPaddingRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-border);

	return result;
}

RectF Gui::GetBorderRect()
{
	RectF result = GetRect();

	result.MoveSidesLocal(-margin);

	return result;
}

RectF Gui::GetChildrenRect()
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

bool Gui::IsChild(Gui* gui)
{
	int idx = gui->GetIndexInParent();

	if (idx >= 0 && idx < GetChildren().size())
	{
		Gui* potentialChild = GetChildren()[idx];

		return potentialChild == gui;
	}

	return false;
}

bool Gui::IsSibling(Gui* gui)
{
	Gui* parent0 = GetParent();
	Gui* parent1 = gui->GetParent();

	return (parent0 == parent1) && parent0; // Important if parent0 and parent1 are nullptrs they are not siblings !!
}

Gui* Gui::AddGui()
{
	Gui* p = new Gui(guiEngine);
	Add(p);
	return p;
}

GuiText* Gui::AddText()
{
	GuiText* p = new GuiText(guiEngine);
	Add(p);
	return p;
}

GuiButton* Gui::AddButton()
{
	GuiButton* p = new GuiButton(guiEngine);
	Add(p);
	return p;
}

GuiList* Gui::AddList()
{
	GuiList* p = new GuiList(guiEngine);
	Add(p);
	return p;
}

GuiMenu* Gui::AddMenu()
{
	GuiMenu* p = new GuiMenu(guiEngine);
	Add(p);
	return p;
}

GuiSlider* Gui::AddSlider()
{
	GuiSlider* p = new GuiSlider(guiEngine);
	Add(p);
	return p;
}

GuiCollapsable* Gui::AddCollapsable()
{
	GuiCollapsable* p = new GuiCollapsable(guiEngine);
	Add(p);
	return p;
}

GuiSplitter* Gui::AddSplitter()
{
	GuiSplitter* p = new GuiSplitter(guiEngine);
	Add(p);
	return p;
}

float Gui::GetCursorPosContentSpaceX()
{
	return guiEngine->GetCursorPosX() - pos.x();
}

float Gui::GetCursorPosContentSpaceY()
{
	return guiEngine->GetCursorPosY() - pos.y();
}

bool Gui::IsCursorInside()
{
	return GetRect().IsPointInside(guiEngine->GetCursorPos());
}

Vector2f Gui::GetCursorPosContentSpace()
{
	return guiEngine->GetCursorPos() - GetContentPos();
}

void Gui::BringToFront()
{
	RemoveFromParent(); // Remove from previous layer
	guiEngine->GetPostProcessLayer()->Add(this); // Add to post process (top most layer)
}

Gui* Gui::AsPlane() 
{ 
	assert(Is<Gui>());
	return (Gui*)this;
}

GuiText* Gui::AsText() 
{ 
	assert(Is<GuiText>());
	return (GuiText*)this; 
}

GuiButton* Gui::AsButton() 
{ 
	assert(Is<GuiButton>());
	return (GuiButton*)this;
}

GuiList* Gui::AsList() 
{ 
	assert(Is<GuiList>());
	return (GuiList*)this;
}

GuiSlider* Gui::AsSlider()
{ 
	assert(Is<GuiSlider>());
	return (GuiSlider*)this;
}

GuiCollapsable* Gui::AsCollapsable() 
{ 
	assert(Is<GuiCollapsable>());
	return (GuiCollapsable*)this;
}

GuiSplitter* Gui::AsSplitter() 
{ 
	assert(Is<GuiSplitter>());
	return (GuiSplitter*)this;
}

GuiMenu* Gui::AsMenu()
{ 
	assert(Is<GuiMenu>());
	return (GuiMenu*)this;
}