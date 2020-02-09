#include "DebugInfoFrame.hpp"
#include "GameSceneFrame.hpp"
#include "PauseMenuFrame.hpp"

#include <BaseLibrary/Platform/System.hpp>


PauseMenuFrame::PauseMenuFrame() {
	m_layout.SetDirection(inl::gui::LinearLayout::VERTICAL);
	m_layout.SetInverted(true);
	SetLayout(m_layout);
	m_layout.AddChild(m_continueButton);
	m_layout[&m_continueButton].SetAuto();
	m_layout.AddChild(m_quickSaveButton);
	m_layout[&m_quickSaveButton].SetAuto();
	m_layout.AddChild(m_quickLoadButton);
	m_layout[&m_quickLoadButton].SetAuto();
	m_layout.AddChild(m_toggleDebugInfoButton);
	m_layout[&m_toggleDebugInfoButton].SetAuto();
	m_layout.AddChild(m_spacer);
	m_layout[&m_spacer].SetWeight(1);
	m_layout.AddChild(m_quitButton);
	m_layout[&m_quitButton].SetAuto();

	m_continueButton.SetText(U"Continue");
	m_continueButton.OnClick += [this](auto...) {
		GetCompositor().HideFrame<PauseMenuFrame>();
	};
	
	m_quickSaveButton.SetText(U"Quick Save");

	m_quickSaveButton.OnClick += [this](auto...) {
		OnSave(inl::System::GetExecutableDir() / "Savegames" / "quicksave.json");
	};
	
	m_quickLoadButton.SetText(U"Quick Load");

	m_quickLoadButton.OnClick += [this](auto...) {
		OnLoad(inl::System::GetExecutableDir() / "Savegames" / "quicksave.json");
	};

	m_toggleDebugInfoButton.SetText(U"Toggle Info");
	m_toggleDebugInfoButton.OnClick += [this](auto...) {
		bool isShown = GetCompositor().IsFrameVisible<DebugInfoFrame>();
		if (isShown) {
			GetCompositor().HideFrame<DebugInfoFrame>();
		}
		else {
			GetCompositor().ShowFrame<DebugInfoFrame>();
		}
	};

	m_quitButton.SetText(U"Quit");
	m_quitButton.OnClick += [this](auto...) {
		OnQuit();
	};

	SetSize({ 200, 300 });
	SetPosition({ 0, 0 });
}
