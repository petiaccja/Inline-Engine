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

Gui* Gui::AddGui()
{
	return Add<Gui>();
}

GuiText* Gui::AddText()
{
	return Add<GuiText>();
}

GuiButton* Gui::AddButton()
{
	return Add<GuiButton>();
}

GuiList* Gui::AddList()
{
	return Add<GuiList>();
}

GuiMenu* Gui::AddMenu()
{
	return Add<GuiMenu>();
}

GuiSlider* Gui::AddSlider()
{
	return Add<GuiSlider>();
}

GuiCollapsable* Gui::AddCollapsable()
{
	return Add<GuiCollapsable>();
}

GuiSplitter* Gui::AddSplitter()
{
	return Add<GuiSplitter>();
}


Gui* Gui::CreateGui()
{
	return new Gui(guiEngine);
}

GuiText* Gui::CreateText()
{
	return new GuiText(guiEngine);
}

GuiButton* Gui::CreateButton()
{
	return new GuiButton(guiEngine);
}

GuiList* Gui::CreateList()
{
	return new GuiList(guiEngine);
}

GuiMenu* Gui::CreateMenu()
{
	return new GuiMenu(guiEngine);
}

GuiSlider* Gui::CreateSlider()
{
	return new GuiSlider(guiEngine);
}

GuiCollapsable* Gui::CreateCollapsable()
{
	return new GuiCollapsable(guiEngine);
}

GuiSplitter* Gui::CreateSplitter()
{
	return new GuiSplitter(guiEngine);
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
	Remove(); // Remove from previous layer
	guiEngine->GetPostProcessLayer()->Add(this); // Add to post process (top most layer)
}