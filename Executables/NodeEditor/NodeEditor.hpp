#pragma once

#include "GraphController.hpp"
#include "NodePanel.hpp"
#include "NodeSelectPanel.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/Board.hpp>
#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/LinearLayout.hpp>

namespace inl::tool {

class NodeEditor {
public:
	NodeEditor(gxeng::IGraphicsEngine* engine, Window* window, std::vector<IEditorGraph*> editors);

	void Update(float elapsed);

private:
	// Window event handlers.
	void OnResize(ResizeEvent evt);

private:
	void CreateGraphicsEnvironment();
	void CreateGui();

	void SetNodeList(const IEditorGraph* editor);
	void NewGraph(IEditorGraph* editor);

	void LoadGraph(const std::filesystem::path& filePath);
	void SaveGraph(const std::filesystem::path& filePath) const;

	std::optional<std::string> ShowLoadDialog() const;
	std::optional<std::string> ShowSaveDialog() const;

private:
	gxeng::IGraphicsEngine* m_engine;
	Window* m_window;
	std::vector<IEditorGraph*> m_editors;

	std::unique_ptr<gxeng::ICamera2D> m_camera;
	std::unique_ptr<gxeng::IScene> m_scene;
	std::shared_ptr<gxeng::IFont> m_font;

	gui::Board m_board;
	gui::Frame m_mainFrame;
	gui::LinearLayout m_mainLayout;

	GraphController m_controller;
	NodePanel m_nodePanel;

	NodeSelectPanel m_selectPanel;

	gui::LinearLayout m_sidePanelLayout;
	gui::Button m_saveButton;
	gui::Button m_openButton;
	std::vector<gui::Button> m_newButtons;
};



} // namespace inl::tool
