#pragma once

#include <GraphicsEngine/Scene/IScene.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>

#include <BaseLibrary/Platform/Input.hpp>
#include <BaseLibrary/Color.hpp>
#include <InlineMath.hpp>

#include "Node.hpp"

#include <any>
#include "SelectPanel.hpp"


namespace inl::tool {


class NodeEditor {
public:
	NodeEditor(gxeng::IGraphicsEngine* graphicsEngine, std::vector<IEditorGraph*> availableEditors);

	void Update();

	void OnResize(ResizeEvent evt);
	void OnMouseMove(MouseMoveEvent evt);
	void OnMouseWheel(MouseWheelEvent evt);
	void OnMouseClick(MouseButtonEvent evt);
	void OnKey(KeyboardEvent evt);

private:
	const Drawable* Intersect(Vec2 position) const;
	const Link* IntersectLinks(Vec2 position) const;


	void Hightlight(const Drawable* target, ColorF color);
	void RemoveHighlight();

	void SelectNode(Node* node);
	void SelectPort(Port* port);

	void EnableDrag(Vec2 offset);
	void DisableDrag();

	void EnableLink();
	void DisableLink();

	void EnablePlacing();
	void DisablePlacing();

	void EnablePan();
	void DisablePan();

	void SendToBack(Node* node);
	void SendToFront(Node* node);

	void DeleteNode(Node* node);
	void Clear();

	void ErrorMessage(std::string msg);

	Vec2 ScreenToWorld(Vec2 screenPoint) const;

	void OpenFile(std::string path);
	void SaveFile(std::string path);

	static std::optional<std::string> OpenDialog();
	static std::optional<std::string> SaveDialog();

	int FindNodeIndex(Node* node) const;
private:
	Node* m_selectedNode = nullptr;
	bool m_enableDrag = false;
	Vec2 m_dragOffset = {0.0f,0.0f};

	Port* m_selectedPort = nullptr;
	bool m_enableLink = false;

	bool m_placing = false;
	SelectItem* m_highlightedItem = nullptr;

	bool m_enablePan = false;
private:
	gxeng::IGraphicsEngine* m_graphicsEngine;
	IEditorGraph* m_graphEditor;
	std::vector<IEditorGraph*> m_availableEditors;

	std::unique_ptr<gxeng::IScene> m_scene;
	std::unique_ptr<gxeng::IFont> m_font;
	std::unique_ptr<gxeng::ICamera2D> m_camera;

	std::unique_ptr<gxeng::IOverlayEntity> m_highlight1, m_highlight2, m_background;

	std::vector<std::unique_ptr<Node>> m_nodes;
	std::unique_ptr<SelectPanel> m_selectPanel;
};


} // namespace inl::tool