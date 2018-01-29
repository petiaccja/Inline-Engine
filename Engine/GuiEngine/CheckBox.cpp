#include "CheckBox.hpp"

using namespace inl::gui;

CheckBox::CheckBox(GuiEngine& guiEngine)
    :Gui(guiEngine),
    eState(CheckState::eUnchecked)
{
    setupUi();
    connectSlot();
}

CheckBox& CheckBox::operator = (const CheckBox& other)
{
    Gui::operator = (other);

    text = Copy(other.text);
    SetCheckState(other.GetState());
    return *this;
}

void CheckBox::SetText(const std::wstring& str)
{
    text->SetText(str);
}

void CheckBox::SetText(const std::string & str)
{
    text->SetText(str);
}

void CheckBox::SetCheckState(CheckState state)
{
    eState = state;
    if (eState == CheckState::eChecked)
    {
        checkBoxImage->SetBgIdleImage(L"Resources/unchecked.png", 20, 20);
    }
    else if (eState == CheckState::eUnchecked)
    {
        checkBoxImage->SetBgIdleImage(L"Resources/checked.png", 20, 20);
    }
    onStateChange();
}

void CheckBox::setupUi()
{
    checkBoxImage = new Image(guiEngine);
    checkBoxImage->SetImage(L"Resources/unchecked.png", 20, 20);
    checkBoxImage->DisableHover();
    checkBoxImage->AlignCenter();

    text = new Text(guiEngine);
    text->AlignCenter();
    text->DisableHover();

    layout = AddGui<Grid>();
    layout->SetHeight(20);
    layout->SetPadding(1);
    layout->StretchFillParent();
    layout->SetDimension(2, 1);

    layout->GetRow(0)->StretchFitToContent();
    layout->GetColumn(0)->SetWidth(20);
    layout->GetCell(0, 0)->AddGui(checkBoxImage);

    layout->GetColumn(1)->StretchFitToContent();
    layout->GetCell(1, 0)->AddGui(text);
}
void CheckBox::connectSlot()
{
    this->OnCursorClick += [](Gui& self, CursorEvent& evt)
    {
        CheckBox& checkBox = self.As<CheckBox>();
        CheckState toggleState = checkBox.GetState() == CheckState::eChecked ? CheckState::eUnchecked : CheckState::eChecked;
        checkBox.SetCheckState(toggleState);
    };
}
