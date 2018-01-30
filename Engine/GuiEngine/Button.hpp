#pragma once
#include "Text.hpp"

namespace inl::gui {

class Button : public Gui
{
public:
	Button(GuiEngine* guiEngine);
	Button(const Button& other):Gui(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual Button* Clone() const override { return new Button(*this); }
	Button& operator = (const Button& other);

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);

	Text* GetText() { return text; }

public:
	Text* text;
};

} // namespace inl::gui
