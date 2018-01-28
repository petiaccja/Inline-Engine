#include "GuiCheckBox.hpp"

using namespace inl::gui;

GuiCheckBox::GuiCheckBox(GuiEngine& guiEngine)
    :Gui(guiEngine)
{
    checkBox = AddGui<GuiButton>();
    checkBox->SetText(L"✓");
    checkBox->AlignLeft();
    checkBox->StretchFitToContent();
    text = AddGui<GuiText>();
    text->SetPosX(checkBox->GetPosX() + checkBox->GetWidth());
    text->AlignVerCenter();
}

GuiCheckBox& GuiCheckBox::operator = (const GuiCheckBox& other)
{
    Gui::operator = (other);

    text = Copy(other.text);

    return *this;
}

void GuiCheckBox::SetText(const std::wstring& str)
{
    text->SetText(str);
}

void inl::gui::GuiCheckBox::SetText(const std::string & str)
{
    text->SetText(str);
}
