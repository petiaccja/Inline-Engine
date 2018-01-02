#pragma once
#include "Gui.hpp"
#include <vector>

namespace inl::ui {

class GuiLayer : public Gui
{
public:
	GuiLayer(GuiEngine& guiEngine);

	virtual Vec2 ArrangeChildren(const Vec2& finalSize);
};

} // namespace inl::ui