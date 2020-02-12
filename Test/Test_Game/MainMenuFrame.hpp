#pragma once

#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


class MainMenuFrame final : public inl::gui::Frame {
public:
	MainMenuFrame();

	inl::Event<> OnStart;
	inl::Event<> OnLoad;
	inl::Event<> OnToggleInfo;
	inl::Event<> OnQuit;

private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Button m_startButton;
	inl::gui::Button m_loadButton;
	inl::gui::Button m_toggleDebugInfoButton;
	inl::gui::Label m_spacer;
	inl::gui::Button m_quitButton;
};
