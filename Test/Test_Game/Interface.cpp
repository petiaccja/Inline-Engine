#include "Interface.hpp"

#include "DebugInfoFrame.hpp"
#include "GameSceneFrame.hpp"
#include "MainMenuFrame.hpp"

#include <fstream>

using namespace inl;

Interface::Interface(const ModuleCollection& modules, inl::Window& window)
	: m_compositor(m_board) {
	m_camera = modules.GetGraphicsEngine().CreateCamera2D("GuiCamera");
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
	m_board.SetDrawingContext(ctx);
	m_board.SetDepth(0.0f);
	inl::gui::ControlStyle style = inl::gui::ControlStyle::Dark(inl::Window::GetWindows10AccentColor().value_or(inl::ColorF{ 0.8f, 0.2f, 0.2f, 1.0f }));
	style.font = m_font.get();
	m_board.SetStyle(style);

	// Set events.
	window.OnKeyboard += Delegate<void(KeyboardEvent)>{ &gui::Board::OnKeyboard, &m_board };
	window.OnMouseButton += Delegate<void(MouseButtonEvent)>{ &gui::Board::OnMouseButton, &m_board };
	window.OnMouseMove += Delegate<void(MouseMoveEvent)>{ &gui::Board::OnMouseMove, &m_board };
	window.OnMouseWheel += Delegate<void(MouseWheelEvent)>{ &gui::Board::OnMouseWheel, &m_board };

	// Show frames.
	DebugInfoFrame& debugInfoFrame = m_compositor.ShowFrame<DebugInfoFrame>();
	MainMenuFrame& mainMenuFrame = m_compositor.ShowFrame<MainMenuFrame>();

	m_compositor.GetBinding(debugInfoFrame).SetAnchors(true, false, false, true);
	debugInfoFrame.SetSize({ 300, 150 });
	debugInfoFrame.SetAdapterInfo(modules.GetGraphicsAdapter());

	m_compositor.GetBinding(mainMenuFrame).SetAnchors(true, true, true, true).SetResizing(false);
	mainMenuFrame.SetSize({ 200, 350 });
	mainMenuFrame.OnQuit += [&window] { window.Close(); };
}


void Interface::operator()(ResizeScreenAction action) {
	ResizeRender(action.width, action.height);
}


void Interface::operator()(UpdateLoadingAction action) {
	
}


void Interface::ResizeRender(int width, int height) {
	m_camera->SetPosition(Vec2(width, height) / 2.0f);
	m_camera->SetExtent(Vec2(width, height));
	m_board.SetCoordinateMapping({ 0.f, (float)width, (float)height, 0.f }, { 0.f, (float)width, 0.f, (float)height });

	m_compositor.SetSize(Vec2(width, height));
	m_compositor.SetPosition(Vec2(width, height) / 2.0f);
	try {
		m_compositor.GetFrame<DebugInfoFrame>().SetResolutionInfo(Vec2(width, height));
	}
	catch (...) {
	}
}
