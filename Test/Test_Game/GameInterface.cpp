#include "GameInterface.hpp"

#include <fstream>


GameInterface::GameInterface(const ModuleCollection& modules) {
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
}

inl::gui::Board& GameInterface::GetBoard() {
	return m_board;
}

inl::gxeng::ICamera2D& GameInterface::GetCamera() {
	return *m_camera;
}