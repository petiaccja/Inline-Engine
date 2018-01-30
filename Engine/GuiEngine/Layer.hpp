#pragma once
#include "Gui.hpp"
#include <vector>

namespace inl::gui {

class Layer : public Gui
{
public:
	Layer(GuiEngine* guiEngine);

	virtual Vec2 ArrangeChildren();
};

} // namespace inl::gui