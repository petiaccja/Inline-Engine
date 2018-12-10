#include "NodeEditor.hpp"


namespace inl::tool {


NodeEditor::NodeEditor(gxeng::IGraphicsEngine* engine, Window* window, std::vector<IEditorGraph*> editors)
	: m_engine(engine), m_window(window), m_editors(std::move(editors)) {
	window->OnResize += Delegate<void(ResizeEvent)>{ &NodeEditor::OnResize, this };

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
	evt.clientSize = Max(evt.clientSize, Vec2u{ 1, 1 });
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

	m_mainLayout.PushBack(m_selectPanel, gui::LinearLayout::CellSize().SetWidth(250));
	m_mainLayout.PushBack(m_nodePanel, gui::LinearLayout::CellSize().SetWeight(1));
	m_mainLayout.PushBack(m_sidePanelLayout, gui::LinearLayout::CellSize().SetWidth(200));


	auto nodes = m_editors[0]->GetNodeList();
	std::vector<std::u32string> u32nodes;
	for (const auto& name : nodes) {
		u32nodes.push_back(EncodeString<char32_t>(name));
	}
	m_selectPanel.SetChoices(u32nodes);

	
	style.background = { 0.08f, 0.08f, 0.08f, 1.0f };
	m_nodePanel.SetStyle(style);

	m_controller.SetSelectPanel(m_selectPanel);
	m_controller.SetNodePanel(m_nodePanel);
	m_controller.SetEditorGraph(*m_editors[0]);

	m_resetButton.SetText(U"Reset");
	m_resetButton.OnClick += [this](auto...) { m_controller.Clear(); };

	m_sidePanelLayout.PushBack(m_resetButton, gui::LinearLayout::CellSize().SetWidth(30.f));

	m_sidePanelLayout.SetDirection(gui::LinearLayout::VERTICAL);
	m_sidePanelLayout.SetInverted(true);

	m_window->OnMouseButton += Delegate<void(MouseButtonEvent)>{ &gui::Board::OnMouseButton, &m_board };
	m_window->OnMouseMove += Delegate<void(MouseMoveEvent)>{ &gui::Board::OnMouseMove, &m_board };
	m_window->OnKeyboard += Delegate<void(KeyboardEvent)>{ &gui::Board::OnKeyboard, &m_board };
	m_window->OnCharacter += Delegate<void(char32_t)>{ &gui::Board::OnCharacter, &m_board };
}


} // namespace inl::tool
