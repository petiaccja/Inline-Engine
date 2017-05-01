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

} // namespace inl::gui