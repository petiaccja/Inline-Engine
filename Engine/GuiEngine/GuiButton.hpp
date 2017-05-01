#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiText.hpp"

namespace inl::gui {

class GuiButton : public Gui
{
public:
	GuiButton(GuiEngine* guiEngine);
	GuiButton(const GuiButton& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiButton* Clone() const override { return new GuiButton(*this); }
	GuiButton& operator = (const GuiButton& other);

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);

	GuiText* GetTextGui() { return text; }

public:
	GuiText* text;
};

} // namespace inl::gui