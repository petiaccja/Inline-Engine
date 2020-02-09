#pragma once

#include "UserInterfaceCompositor.hpp"

#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


class PauseMenuFrame final : public TopLevelFrame, public inl::gui::Frame {
public:
	PauseMenuFrame();

	inl::Event<> OnQuit;
	inl::Event<std::filesystem::path> OnLoad;
	inl::Event<std::filesystem::path> OnSave;

private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Button m_continueButton;
	inl::gui::Button m_quickSaveButton;
	inl::gui::Button m_quickLoadButton;
	inl::gui::Button m_toggleDebugInfoButton;
	inl::gui::Label m_spacer;
	inl::gui::Button m_quitButton;
};
