#pragma once
#include "Gui.hpp"
#include <vector>

namespace inl::gui {

class GuiLayer : public Gui
{
public:
	GuiLayer(GuiEngine* guiEngine);

	virtual Vector2f ArrangeChildren(const Vector2f& finalSize);

protected:
	GuiEngine* guiEngine;
};

inline Vector2f GuiLayer::ArrangeChildren(const Vector2f& finalSize)
{
	for (Gui* child : GetChildren())
		child->Arrange(child->GetPos(), child->GetDesiredSize());

	return GetSize();
}
} // namespace inl::gui