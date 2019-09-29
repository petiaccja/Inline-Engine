#pragma once

#include "UserInterfaceCompositor.hpp"

#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/LinearLayout.hpp>


class MainMenuFrame : public TopLevelFrame, public inl::gui::Frame {
public:
	MainMenuFrame();

private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Button m_startButton;
	inl::gui::Button m_toggleDebugInfoButton;
};
