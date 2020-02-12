#include "MainMenuFrame.hpp"

#include "DebugInfoFrame.hpp"


MainMenuFrame::MainMenuFrame() {
	m_layout.SetDirection(inl::gui::LinearLayout::VERTICAL);
	m_layout.SetInverted(true);
	SetLayout(m_layout);
	m_layout.AddChild(m_startButton);
	m_layout[&m_startButton].SetAuto();
	m_layout.AddChild(m_loadButton);
	m_layout[&m_loadButton].SetAuto();
	m_layout.AddChild(m_toggleDebugInfoButton);
	m_layout[&m_toggleDebugInfoButton].SetAuto();
	m_layout.AddChild(m_spacer);
	m_layout[&m_spacer].SetWeight(1);
	m_layout.AddChild(m_quitButton);
	m_layout[&m_quitButton].SetAuto();

	m_startButton.SetText(U"Start");
	m_startButton.OnClick += [this](auto...) {
		OnStart();
	};

	m_loadButton.SetText(U"Load");
	m_loadButton.OnClick += [this](auto...) {
		OnLoad();
	};

	m_toggleDebugInfoButton.SetText(U"Toggle Info");
	m_toggleDebugInfoButton.OnClick += [this](auto...) {
		OnToggleInfo();
	};

	m_quitButton.SetText(U"Quit");
	m_quitButton.OnClick += [this](auto...) {
		OnQuit();
	};
}
