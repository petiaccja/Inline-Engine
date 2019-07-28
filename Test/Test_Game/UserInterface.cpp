#include "UserInterface.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <fstream>


UserInterface::UserInterface(inl::gxeng::IGraphicsEngine& engine, inl::gxeng::IScene& scene, inl::gxeng::ICamera2D& camera)
	: m_context{ &engine, &scene }, m_camera(camera)

{
	m_board.AddChild(m_layout);
	m_layout.SetDepth(0.0f);
	m_layout.SetYDown(false);
	m_layout.SetReferencePoint(inl::gui::AbsoluteLayout::eRefPoint::BOTTOMLEFT);
	m_board.SetDrawingContext(m_context);

	m_font.reset(engine.CreateFont());

	std::ifstream fontFile;
	fontFile.open(R"(C:\Windows\Fonts\calibri.ttf)", std::ios::binary);
	m_font->LoadFile(fontFile);

	inl::gui::ControlStyle style = inl::gui::ControlStyle::Dark(inl::ColorF(0.2f, 0.2f, 0.7f));
	style.font = m_font.get();
	m_board.SetStyle(style);
}


void UserInterface::Update(float elapsed) {
	m_board.Update(elapsed);
}


void UserInterface::AddFrame(inl::gui::Frame& frame) {
	m_layout.AddChild(frame);
}


void UserInterface::RemoveFrame(inl::gui::Frame& frame) {
	m_layout.RemoveChild(&frame);
}


void UserInterface::SetResolution(inl::Vec2u windowSize, inl::Vec2u renderSize) {
	m_board.SetCoordinateMapping({ 0.f, (float)windowSize.x, (float)windowSize.y, 0.f }, { 0.f, (float)renderSize.x, 0.f, (float)renderSize.y });
	m_camera.SetPosition(inl::Vec2(renderSize) / 2.0f);
	m_camera.SetExtent(renderSize);
	m_camera.SetRotation(0.0f);
	m_camera.SetVerticalFlip(false);
	m_layout.SetPosition(inl::Vec2(renderSize) / 2.0f);
	m_layout.SetSize(renderSize);
}
