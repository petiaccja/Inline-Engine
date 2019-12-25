#include "UserInterfaceSystem.hpp"

#include "DebugInfoFrame.hpp"
#include "MainMenuFrame.hpp"

#include <BaseLibrary/Event.hpp>
#include <BaseLibrary/Platform/Window.hpp>

#include <fstream>


UserInterfaceSystem::UserInterfaceSystem(const ModuleCollection& modules, inl::Window& window)
	: m_window(window), m_board(std::make_unique<inl::gui::Board>()), m_compositor(std::make_unique<UserInterfaceCompositor>(*m_board)) {
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

	// Show frames.
	DebugInfoFrame& debugInfoFrame = m_compositor->ShowFrame<DebugInfoFrame>();
	MainMenuFrame& mainMenuFrame = m_compositor->ShowFrame<MainMenuFrame>();

	m_compositor->GetBinding(debugInfoFrame).SetAnchors(true, false, false, true);
	debugInfoFrame.SetSize({ 300, 150 });
	debugInfoFrame.SetAdapterInfo(modules.GetGraphicsAdapter());

	m_compositor->GetBinding(mainMenuFrame).SetAnchors(true, true, true, true).SetResizing(false);
	mainMenuFrame.SetSize({ 200, 350 });
	mainMenuFrame.OnQuit += [&window] { window.Close(); };

	// Set events.
	RegisterBoardEvents();
	RegisterWindowEvents();
}

UserInterfaceSystem::UserInterfaceSystem(UserInterfaceSystem&& rhs) : m_window(rhs.m_window) {
	m_font = std::move(rhs.m_font);
	m_scene = std::move(rhs.m_scene);
	m_camera = std::move(rhs.m_camera);
	m_board = std::move(rhs.m_board);
	m_compositor = std::move(rhs.m_compositor);
	RegisterWindowEvents();
}

UserInterfaceSystem::~UserInterfaceSystem() noexcept {
	UnregisterBoardEvents();
	UnregisterWindowEvents();
}

void UserInterfaceSystem::Update(float elapsed) {
	m_window.CallEvents();
	m_board->Update(elapsed);
}


void UserInterfaceSystem::ResizeRender(inl::ResizeEvent evt) {
	int width = evt.clientSize.x;
	int height = evt.clientSize.y;

	m_camera->SetPosition(inl::Vec2(width, height) / 2.0f);
	m_camera->SetExtent(inl::Vec2(width, height));
	m_board->SetCoordinateMapping({ 0.f, (float)width, (float)height, 0.f }, { 0.f, (float)width, 0.f, (float)height });

	m_compositor->SetSize(inl::Vec2(width, height));
	m_compositor->SetPosition(inl::Vec2(width, height) / 2.0f);
	try {
		m_compositor->GetFrame<DebugInfoFrame>().SetResolutionInfo(inl::Vec2(width, height));
	}
	catch (...) {
	}
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
}

void UserInterfaceSystem::UnregisterWindowEvents() {
	m_window.OnResize -= inl::Delegate<void(inl::ResizeEvent)>{ &UserInterfaceSystem::ResizeRender, this };
}
