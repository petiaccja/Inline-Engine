#pragma once
#include "BaseLibrary\Common.hpp"
#include "GuiText.hpp"

namespace inl::gui {

class GuiButton : public Gui
{
public:
	GuiButton(GuiEngine& guiEngine);
	GuiButton(const GuiButton& other):Gui(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual GuiButton* Clone() const override { return new GuiButton(*this); }
	GuiButton& operator = (const GuiButton& other);

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);

	GuiText* GetGuiText() { return text; }

public:
	GuiText* text;
};

} // namespace inl::gui