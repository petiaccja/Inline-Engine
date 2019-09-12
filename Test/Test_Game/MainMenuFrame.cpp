#include "MainMenuFrame.hpp"
#include <iostream> // debug only


MainMenuFrame::MainMenuFrame() {
	m_layout.SetDirection(inl::gui::LinearLayout::VERTICAL);
	m_layout.SetInverted(true);
	SetLayout(m_layout);
	m_layout.AddChild(m_startButton);
	m_layout[&m_startButton].SetAuto();

	m_startButton.SetText(U"Start game");

	m_startButton.OnClick += [](auto, auto, auto) {
		std::cout << "asd" << std::endl;
	};
}
