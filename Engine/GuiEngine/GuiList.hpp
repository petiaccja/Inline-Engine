#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Gui.hpp"

namespace inl::gui {

enum class eGuiListDirection
{
	VERTICAL,
	HORIZONTAL,
};

class GuiList : public Gui
{
public:
	GuiList(GuiEngine* guiEngine);
	GuiList(const GuiList& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	void SetDirection(eGuiListDirection dir);
	eGuiListDirection GetDirection() { return direction; }

protected:
	//void MeasureContentChilds();

	virtual Vector2f MeasureChildren(const Vector2f& availableSize) override;
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	eGuiListDirection direction;
};

inline GuiList::GuiList(GuiEngine* guiEngine)
:Gui(guiEngine), direction(eGuiListDirection::VERTICAL)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

inline void GuiList::SetDirection(eGuiListDirection dir)
{
	direction = dir;
}

inline Vector2f GuiList::MeasureChildren(const Vector2f& availableSize)
{
	Vector2f finalSize(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f childSize = child->Measure(Vector2f(FLT_MAX, FLT_MAX));

		if (direction == eGuiListDirection::VERTICAL)
		{
			finalSize.y() += childSize.y();
			finalSize.x() = std::max(finalSize.x(), childSize.x());
		}
		else if (direction == eGuiListDirection::HORIZONTAL)
		{
			finalSize.x() += childSize.x();
			finalSize.y() = std::max(finalSize.y(), childSize.y());
		}
	}

	return finalSize;
}

inline Vector2f GuiList::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f pos = GetContentPos();
	Vector2f size(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f desiredSize = child->desiredSize;

		if (direction == eGuiListDirection::VERTICAL)
		{
			child->StretchFillParentHor();
			child->Arrange(pos.x(), pos.y() + size.y(), desiredSize);

			size.y() += desiredSize.y();
			size.x() = std::max(size.x(), desiredSize.x());
		}
		else if (direction == eGuiListDirection::HORIZONTAL)
		{
			child->StretchFillParentVer();
			child->Arrange(GetContentPosX() + size.x(), GetContentPosY(), desiredSize);

			size.x() += desiredSize.x();
			size.y() = std::max(size.y(), desiredSize.y());
		}
	}

	return size;
}

} // namespace inl::gui