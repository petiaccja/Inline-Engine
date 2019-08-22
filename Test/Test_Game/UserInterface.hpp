#pragma once

#include <GraphicsEngine/Resources/IFont.hpp>
#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Board.hpp>
#include <GuiEngine/Frame.hpp>


namespace inl {
class Window;
}


class UserInterface {
public:
	UserInterface(inl::gxeng::IGraphicsEngine& engine, inl::gxeng::IScene& scene, inl::gxeng::ICamera2D& camera);

	void Update(float elapsed);

	void AddFrame(inl::gui::Frame& frame);
	void RemoveFrame(inl::gui::Frame& frame);

	auto& operator[](inl::gui::Frame& frame) {
		return m_layout[&frame];
	}
	const auto& operator[](inl::gui::Frame& frame) const {
		return m_layout[&frame];
	}

	void SetResolution(inl::Vec2u windowSize, inl::Vec2u renderSize);

	inl::gui::Board& GetBoard();

private:
	inl::gui::GraphicsContext m_context;
	inl::gui::Board m_board;
	inl::gui::AbsoluteLayout m_layout;
	std::unique_ptr<inl::gxeng::IFont> m_font;
	inl::gxeng::ICamera2D& m_camera;
};
