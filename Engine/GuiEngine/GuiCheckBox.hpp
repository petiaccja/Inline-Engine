#pragma once
#include "GuiText.hpp"
#include "GuiButton.hpp"

namespace inl::gui {

    enum CheckState
    {
        eUnchecked,
        ePartiallyChecked,
        eChecked,
    };

    class GuiCheckBox : public Gui
    {
    public:
        GuiCheckBox(GuiEngine& guiEngine);
        GuiCheckBox(const GuiCheckBox& other) :Gui(other.guiEngine) { *this = other; }

	    // Important to implement in derived classes
        virtual GuiCheckBox* Clone() const override { return new GuiCheckBox(*this); };
        GuiCheckBox& operator = (const GuiCheckBox& other);

        void setCheckState(CheckState state) { eState = state; }
	    void SetText(const std::wstring& str);
        void SetText(const std::string& str);

	    GuiText* GetText() { return text; }
        CheckState checkState() const { return eState; }
    public:
	    GuiText * text;
        GuiButton *checkBox;
        CheckState eState;
    };
}