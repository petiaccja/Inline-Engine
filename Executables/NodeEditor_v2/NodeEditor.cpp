#include "NodeEditor.hpp"


namespace inl::tool {


NodeEditor::NodeEditor(gxeng::IGraphicsEngine* engine, Window* window) 
	: m_engine(engine), m_window(window)
{
	window->OnResize += Delegate<void(ResizeEvent)>{&NodeEditor::OnResize, this};

	CreateGraphicsEnvironment();
	CreateGui();

	ResizeEvent fakeEvent;
	fakeEvent.clientSize = window->GetClientSize();
	fakeEvent.size = window->GetSize();
	fakeEvent.resizeMode = eResizeMode::RESTORED;
	OnResize(fakeEvent);
}

void NodeEditor::Update(float elapsed) {
	m_board.Update(elapsed);
}


void NodeEditor::OnResize(ResizeEvent evt) {
	evt.clientSize = Max(evt.clientSize, Vec2u{ 1,1 });
	m_engine->SetScreenSize(evt.clientSize.x, evt.clientSize.y);

	m_camera->SetPosition(Vec2(evt.clientSize) / 2.f);
	m_camera->SetExtent(evt.clientSize);

	m_mainFrame.SetPosition(m_camera->GetPosition());
	m_mainFrame.SetSize(m_camera->GetExtent());

	m_board.SetCoordinateMapping({ 0.f, (float)evt.clientSize.x, (float)evt.clientSize.y, 0.f }, { 0.f, (float)evt.clientSize.x, 0.f, (float)evt.clientSize.y });
}


void NodeEditor::CreateGraphicsEnvironment() {
	m_camera.reset(m_engine->CreateCamera2D("GuiCam"));
	m_scene.reset(m_engine->CreateScene("GuiScene"));
	m_font.reset(m_engine->CreateFont());

	std::ifstream fontFile;
	fontFile.open(R"(C:\Windows\Fonts\calibri.ttf)", std::ios::binary);
	m_font->LoadFile(fontFile);
}

void NodeEditor::CreateGui() {
	gui::DrawingContext drawingContext;
	drawingContext.engine = m_engine;
	drawingContext.scene = m_scene.get();
	m_board.SetDrawingContext(drawingContext);
	gui::ControlStyle style;
	style.font = m_font.get();
	m_board.SetStyle(style);
	
	m_board.AddControl(m_mainFrame);

	m_mainFrame.SetLayout(m_mainLayout);

	m_mainLayout.SetNumCells(3);
	m_mainLayout[0].SetWeight(1);

	m_mainLayout[1].SetControl(m_mainLayoutSep);
	m_mainLayout[1].SetWidth(8);

	m_mainLayout[2].SetWidth(200);
	m_mainLayout[2].SetControl(m_sidePanelLayout);

	m_sidePanelDummy1.SetText("Default");
	m_sidePanelDummy2.SetText("Clear");
	m_sidePanelDummy1.OnClick += [this](auto, auto) { m_sidePanelDummy3.SetText("Default"); };
	m_sidePanelDummy2.OnClick += [this](auto, auto) { m_sidePanelDummy3.SetText({}); };
	m_sidePanelLayout.AddChild(m_sidePanelDummy1, 0);
	m_sidePanelLayout.AddChild(m_sidePanelDummy2, 1);
	m_sidePanelLayout.AddChild(m_sidePanelDummy3, 2);
	m_sidePanelLayout[0].SetWidth(30.f);
	m_sidePanelLayout[1].SetWidth(30.f);
	m_sidePanelLayout[2].SetWidth(30.f);
	m_sidePanelLayout.SetVertical(true);
	m_sidePanelLayout.SetInverted(true);

	gui::ControlStyle sepStyle;
	sepStyle.background = sepStyle.accent;
	m_mainLayoutSep.SetStyle(sepStyle);

	m_mainLayout.SetZOrder(0);
	m_mainLayoutSep.SetZOrder(1);

	m_window->OnMouseButton += Delegate<void(MouseButtonEvent)>{ &gui::Board::OnMouseButton, &m_board };
	m_window->OnMouseMove += Delegate<void(MouseMoveEvent)>{ &gui::Board::OnMouseMove, &m_board };
	m_window->OnKeyboard += Delegate<void(KeyboardEvent)>{ &gui::Board::OnKeyboard, &m_board };
	m_window->OnCharacter += Delegate<void(char32_t)>{ &gui::Board::OnCharacter, &m_board };
}


}
