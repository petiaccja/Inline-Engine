#pragma once

#include "UserInterfaceCompositor.hpp"

#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


class MainMenuFrame final : public TopLevelFrame, public inl::gui::Frame {
public:
	MainMenuFrame();

	inl::Event<> OnQuit;
	inl::Event<> OnStart;

private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Button m_startButton;
	inl::gui::Button m_toggleDebugInfoButton;
	inl::gui::Label m_spacer;
	inl::gui::Button m_quitButton;
};
