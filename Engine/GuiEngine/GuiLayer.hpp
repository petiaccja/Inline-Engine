#pragma once
#include "Gui.hpp"
#include <vector>

namespace inl::gui {

class GuiLayer : public Gui
{
public:
	GuiLayer(GuiEngine& guiEngine);

	virtual Vec2 ArrangeChildren();
};

} // namespace inl::gui