#pragma once
#include "Text.hpp"
#include "Button.hpp"

namespace inl::gui {

    enum CheckState
    {
        eUnchecked,
        ePartiallyChecked,
        eChecked,
    };

    class CheckBox : public Gui
    {
    public:
        CheckBox(GuiEngine& guiEngine);
        CheckBox(const CheckBox& other) :Gui(other.guiEngine) { *this = other; }

	    // Important to implement in derived classes
        virtual CheckBox* Clone() const override { return new CheckBox(*this); };
        CheckBox& operator = (const CheckBox& other);

        void setCheckState(CheckState state) { eState = state; }
	    void SetText(const std::wstring& str);
        void SetText(const std::string& str);

	    Text* GetText() { return text; }
        CheckState checkState() const { return eState; }
    public:
	    Text * text;
        Button *checkBox;
        CheckState eState;
    };
}