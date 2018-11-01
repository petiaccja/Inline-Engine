#pragma once

#include "NodeControl.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Board.hpp>
#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/LinearLayout.hpp>
#include <GuiEngine/TextBox.hpp>

namespace inl::tool {

class NodeEditor {
public:
	NodeEditor(gxeng::IGraphicsEngine* engine, Window* window);

	void Update(float elapsed);

private:
	// Window event handlers.
	void OnResize(ResizeEvent evt);

private:
	void CreateGraphicsEnvironment();
	void CreateGui();

private:
	gxeng::IGraphicsEngine* m_engine;
	Window* m_window;

	std::unique_ptr<gxeng::ICamera2D> m_camera;
	std::unique_ptr<gxeng::IScene> m_scene;
	std::unique_ptr<gxeng::IFont> m_font;

	gui::Board m_board;
	gui::Frame m_mainFrame;
	gui::LinearLayout m_mainLayout;
	gui::Frame m_mainLayoutSep;

	gui::Frame m_testNodeFrame;
	gui::AbsoluteLayout m_testNodeLayout;
	NodeControl m_testNode;

	gui::LinearLayout m_sidePanelLayout;
	gui::Button m_sidePanelDummy1, m_sidePanelDummy2;
	gui::TextBox m_sidePanelDummy3;
};



} // namespace inl::tool
