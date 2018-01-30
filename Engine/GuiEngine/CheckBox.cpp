#include "CheckBox.hpp"

using namespace inl::gui;

CheckBox::CheckBox(GuiEngine* guiEngine)
    :Gui(guiEngine)
{
    checkBox = AddGui<Button>();
    checkBox->SetText(L"✓");
    checkBox->AlignLeft();
    checkBox->StretchFitToContent();
    text = AddGui<Text>();
    text->SetPosX(checkBox->GetPosX() + checkBox->GetWidth());
    text->AlignVerCenter();
}

CheckBox& CheckBox::operator = (const CheckBox& other)
{
    Gui::operator = (other);

    text = Copy(other.text);

    return *this;
}

void CheckBox::SetText(const std::wstring& str)
{
    text->SetText(str);
}

void inl::gui::CheckBox::SetText(const std::string & str)
{
    text->SetText(str);
}
