#include "CheckBox.hpp"

using namespace inl::gui;

CheckBox::CheckBox(GuiEngine* guiEngine)
    :Gui(guiEngine),
    state(eCheckState::UNCHECKED)
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

void CheckBox::SetCheckState(eCheckState state)
{
    this->state = state;
    if (this->state == eCheckState::CHECKED)
    {
        checkBoxImage->SetBgIdleImage(L"Resources/unchecked.png", 20, 20);
    }
    else if (this->state == eCheckState::UNCHECKED)
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
    this->OnCursorClick += [](CursorEvent& evt)
    {
        CheckBox& checkBox = evt.self->As<CheckBox>();
        eCheckState toggleState = checkBox.GetState() == eCheckState::CHECKED ? eCheckState::UNCHECKED : eCheckState::CHECKED;
        checkBox.SetCheckState(toggleState);
    };
}

