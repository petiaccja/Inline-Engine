#include "MainMenuFrame.hpp"

#include "DebugInfoFrame.hpp"

#include <iostream> // debug only


MainMenuFrame::MainMenuFrame() {
	m_layout.SetDirection(inl::gui::LinearLayout::VERTICAL);
	m_layout.SetInverted(true);
	SetLayout(m_layout);
	m_layout.AddChild(m_startButton);
	m_layout[&m_startButton].SetAuto();
	m_layout.AddChild(m_toggleDebugInfoButton);
	m_layout[&m_toggleDebugInfoButton].SetAuto();

	m_startButton.SetText(U"Start game");
	m_startButton.OnClick += [](auto, auto, auto) {
		std::cout << "asd" << std::endl;
	};

	m_toggleDebugInfoButton.SetText(U"Toggle info");
	m_toggleDebugInfoButton.OnClick += [this](auto, auto, auto) {
		GetCompositor().DeleteFrame<DebugInfoFrame>();
	};

	SetSize({ 200, 300 });
	SetPosition({ 0, 0 });
}
