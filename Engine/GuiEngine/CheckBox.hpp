#pragma once
#include "Text.hpp"

#include "Image.hpp"
#include "Grid.hpp"
#include "Button.hpp"

namespace inl::gui {

    enum class CheckState
    {
        eUnchecked,
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

        void SetCheckState(CheckState state);
        void SetText(const std::wstring& str);
        void SetText(const std::string& str);

        Text* GetText() { return text; }
        CheckState GetState() const { return eState; }

        Event<> onStateChange;
    private:
        void setupUi();
        void connectSlot();

        Grid * layout;
        Text * text;
        Image *checkBoxImage;
        CheckBox(GuiEngine* guiEngine);
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