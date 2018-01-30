#pragma once
#include "Text.hpp"
#include "Image.hpp"
#include "Grid.hpp"

namespace inl::gui {

    enum class eCheckState
    {
        UNCHECKED,
        CHECKED,
    };

    class CheckBox : public Gui
    {
    public:
        CheckBox(GuiEngine* guiEngine);
        CheckBox(const CheckBox& other) :Gui(other.guiEngine) { *this = other; }

        // Important to implement in derived classes
        virtual CheckBox* Clone() const override { return new CheckBox(*this); };
        CheckBox& operator = (const CheckBox& other);

        void SetCheckState(eCheckState state);
        void SetText(const std::wstring& str);
        void SetText(const std::string& str);

        Text* GetText() { return text; }
        eCheckState GetState() const { return this->state; }

        Event<> onStateChange;
    private:
        void setupUi();
        void connectSlot();

        Grid * layout;
        Text * text;
        Image *checkBoxImage;
        eCheckState state;
    };
}