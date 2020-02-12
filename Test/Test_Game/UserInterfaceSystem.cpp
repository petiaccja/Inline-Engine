#include "UserInterfaceSystem.hpp"

#include "LevelActions.hpp"

#include <BaseLibrary/Event.hpp>
#include <BaseLibrary/Platform/Window.hpp>

#include <fstream>


UserInterfaceSystem::UserInterfaceSystem(const EngineCollection& modules, inl::Window& window)
	: m_window(window), m_board(std::make_unique<inl::gui::Board>()) {
	m_camera = modules.GetGraphicsEngine().CreateCamera2D("GuiCam");
	m_font = modules.GetGraphicsEngine().CreateFont();
	m_scene = modules.GetGraphicsEngine().CreateScene("GuiScene");

	// Load font.
	std::ifstream fontFile;
	fontFile.open(R"(C:\Windows\Fonts\calibri.ttf)", std::ios::binary);
	m_font->LoadFile(fontFile);

	// Set rendering context to GUI board.
	inl::gui::GraphicsContext ctx;
	ctx.engine = &modules.GetGraphicsEngine();
	ctx.scene = m_scene.get();
	m_board->SetDrawingContext(ctx);
	m_board->SetDepth(0.0f);
	inl::gui::ControlStyle style = inl::gui::ControlStyle::Dark(inl::Window::GetWindows10AccentColor().value_or(inl::ColorF{ 0.8f, 0.2f, 0.2f, 1.0f }));
	style.font = m_font.get();
	m_board->SetStyle(style);

	// Create GUI elements.
	CreateFrames();
	RegisterHandlers();

	// Set events.
	RegisterBoardEvents();
	RegisterWindowEvents();

	// Misc
	auto info = modules.GetGraphicsAdapter();
	m_controls.debugInfo->SetAdapterInfo(info);
}

UserInterfaceSystem::UserInterfaceSystem(UserInterfaceSystem&& rhs) : m_window(rhs.m_window) {
	m_font = std::move(rhs.m_font);
	m_scene = std::move(rhs.m_scene);
	m_camera = std::move(rhs.m_camera);
	m_board = std::move(rhs.m_board);
	rhs.UnregisterHandlers();
	m_controls = std::move(rhs.m_controls);
	RegisterWindowEvents();
	RegisterHandlers();
}

UserInterfaceSystem::~UserInterfaceSystem() noexcept {
	UnregisterBoardEvents();
	UnregisterWindowEvents();
}


void UserInterfaceSystem::ReactActions(ActionHeap& actions) {
	m_transientActionHeap = actions;
}

void UserInterfaceSystem::Update(float elapsed) {
	m_window.CallEvents();
	m_board->Update(elapsed);
}

void UserInterfaceSystem::EmitActions(ActionHeap& actions) {
	m_transientActionHeap.reset();
}


void UserInterfaceSystem::ResizeRender(inl::ResizeEvent evt) {
	int width = evt.clientSize.x;
	int height = evt.clientSize.y;

	m_camera->SetPosition(inl::Vec2(width, height) / 2.0f);
	m_camera->SetExtent(inl::Vec2(width, height));
	m_board->SetCoordinateMapping({ 0.f, (float)width, (float)height, 0.f }, { 0.f, (float)width, 0.f, (float)height });

	m_controls.layout->SetSize(inl::Vec2(width, height));
	m_controls.layout->SetPosition(inl::Vec2(width, height) / 2.0f);

	m_controls.debugInfo->SetResolutionInfo(inl::Vec2(width, height));
}

void UserInterfaceSystem::KeyboardShortcuts(inl::KeyboardEvent evt) {
}


void UserInterfaceSystem::RegisterBoardEvents() {
	m_window.OnKeyboard += inl::Delegate<void(inl::KeyboardEvent)>{ &inl::gui::Board::OnKeyboard, m_board.get() };
	m_window.OnMouseButton += inl::Delegate<void(inl::MouseButtonEvent)>{ &inl::gui::Board::OnMouseButton, m_board.get() };
	m_window.OnMouseMove += inl::Delegate<void(inl::MouseMoveEvent)>{ &inl::gui::Board::OnMouseMove, m_board.get() };
	m_window.OnMouseWheel += inl::Delegate<void(inl::MouseWheelEvent)>{ &inl::gui::Board::OnMouseWheel, m_board.get() };
}

void UserInterfaceSystem::UnregisterBoardEvents() {
	m_window.OnKeyboard -= inl::Delegate<void(inl::KeyboardEvent)>{ &inl::gui::Board::OnKeyboard, m_board.get() };
	m_window.OnMouseButton -= inl::Delegate<void(inl::MouseButtonEvent)>{ &inl::gui::Board::OnMouseButton, m_board.get() };
	m_window.OnMouseMove -= inl::Delegate<void(inl::MouseMoveEvent)>{ &inl::gui::Board::OnMouseMove, m_board.get() };
	m_window.OnMouseWheel -= inl::Delegate<void(inl::MouseWheelEvent)>{ &inl::gui::Board::OnMouseWheel, m_board.get() };
}

void UserInterfaceSystem::RegisterWindowEvents() {
	m_window.OnResize += inl::Delegate<void(inl::ResizeEvent)>{ &UserInterfaceSystem::ResizeRender, this };
	m_window.OnKeyboard += inl::Delegate<void(inl::KeyboardEvent)>{ &UserInterfaceSystem::KeyboardShortcuts, this };
}

void UserInterfaceSystem::UnregisterWindowEvents() {
	m_window.OnResize -= inl::Delegate<void(inl::ResizeEvent)>{ &UserInterfaceSystem::ResizeRender, this };
	m_window.OnKeyboard -= inl::Delegate<void(inl::KeyboardEvent)>{ &UserInterfaceSystem::KeyboardShortcuts, this };
}

void UserInterfaceSystem::CreateFrames() {
	m_controls.layout = std::make_shared<WindowLayout>();

	m_controls.debugInfo = std::make_shared<DebugInfoFrame>();
	m_controls.mainMenu = std::make_shared<MainMenuFrame>();
	m_controls.pauseMenu = std::make_shared<PauseMenuFrame>();
	m_controls.background = std::make_shared<GameSceneFrame>();

	m_controls.layout->AddChild(m_controls.debugInfo);
	m_controls.layout->AddChild(m_controls.mainMenu);
	m_controls.layout->AddChild(m_controls.pauseMenu);
	m_controls.layout->AddChild(m_controls.background);

	(*m_controls.layout)[m_controls.debugInfo.get()].SetAnchors(true, false, false, true).MoveToFront();
	m_controls.debugInfo->SetSize({ 300, 150 });

	(*m_controls.layout)[m_controls.mainMenu.get()].SetAnchors(true, true, true, true).SetResizing(false);
	m_controls.mainMenu->SetSize({ 200, 350 });

	(*m_controls.layout)[m_controls.pauseMenu.get()].SetAnchors(true, true, true, true).SetResizing(false);
	m_controls.pauseMenu->SetSize({ 200, 350 });
	m_controls.pauseMenu->SetVisible(false);

	(*m_controls.layout)[m_controls.background.get()].SetAnchors(true, true, true, true).SetResizing(true).MoveToBack();
	m_controls.background->SetVisible(false);

	m_board->AddChild(m_controls.layout);
}


void UserInterfaceSystem::RegisterHandlers() {
	// Main menu
	m_controls.mainMenu->OnStart += [this] {
		m_transientActionHeap.value().get().Push(LoadTestLevelAction{});
		m_controls.mainMenu->SetVisible(false);
	};
	m_controls.mainMenu->OnLoad += [this] {
		m_transientActionHeap.value().get().Push(LoadLevelAction{R"(D:\Temp\level.json)"});
		m_controls.mainMenu->SetVisible(false);
	};
	m_controls.mainMenu->OnToggleInfo += [this] {
		m_controls.debugInfo->SetVisible(!m_controls.debugInfo->GetVisible());
	};
	m_controls.mainMenu->OnQuit += [this] {
		m_window.Close();
	};
}


void UserInterfaceSystem::UnregisterHandlers() {
	m_controls.mainMenu->OnStart.Clear();
	m_controls.mainMenu->OnLoad.Clear();
	m_controls.mainMenu->OnToggleInfo.Clear();
	m_controls.mainMenu->OnQuit.Clear();
}
