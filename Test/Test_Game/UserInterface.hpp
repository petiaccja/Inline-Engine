#pragma once

#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Board.hpp>
#include <GuiEngine/Frame.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>


namespace inl {
class Window;
}


class UserInterface {
public:
	UserInterface(gxeng::IGraphicsEngine& engine, gxeng::IScene& scene, gxeng::ICamera2D& camera);

	void Update(float elapsed);

	void AddFrame(gui::Frame& frame);
	void RemoveFrame(gui::Frame& frame);

	auto& operator[](gui::Frame& frame) {
		return m_layout[&frame];
	}
	const auto& operator[](gui::Frame& frame) const {
		return m_layout[&frame];
	}

	void SetResolution(Vec2u windowSize, Vec2u renderSize);

private:
	gui::GraphicsContext m_context;
	gui::Board m_board;
	gui::AbsoluteLayout m_layout;
	std::unique_ptr<gxeng::IFont> m_font;
	gxeng::ICamera2D& m_camera;
};
