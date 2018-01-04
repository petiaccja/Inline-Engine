#pragma once
#include "Gui.hpp"

namespace inl::ui
{

class GuiLayout : public Gui
{
public:
	//GuiLayout() {}
	GuiLayout(GuiEngine& guiEngine) :Gui(guiEngine) {}

	virtual void AddItem(Gui* gui) = 0;
	virtual bool RemoveItem(Gui* gui) = 0;
	virtual std::vector<Gui*> GetItems() = 0;

	template<class T>
	T* AddItem()
	{
		T* child = new T(guiEngine);
		AddItem(child);
		return child;
	}
};

} // namespace inl::ui
