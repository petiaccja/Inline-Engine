#pragma once

#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


class PauseMenuFrame final : public inl::gui::Frame {
public:
	PauseMenuFrame();

	inl::Event<> OnContinue;
	inl::Event<> OnToggleInfo;
	inl::Event<> OnSave;
	inl::Event<> OnLoad;
	inl::Event<> OnQuit;

private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Button m_continueButton;
	inl::gui::Button m_quickSaveButton;
	inl::gui::Button m_quickLoadButton;
	inl::gui::Button m_toggleDebugInfoButton;
	inl::gui::Label m_spacer;
	inl::gui::Button m_quitButton;
};
