#pragma once

#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Button.hpp>
#include <GuiEngine/LinearLayout.hpp>



class MainMenuFrame : public inl::gui::Frame {
public:
	MainMenuFrame();
		
private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Button m_startButton;
};
