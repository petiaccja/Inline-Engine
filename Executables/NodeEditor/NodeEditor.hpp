#pragma once

#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/Font.hpp>
#include <GraphicsEngine_LL/Camera2D.hpp>

#include <BaseLibrary/Platform/Input.hpp>
#include <BaseLibrary/Color.hpp>
#include <InlineMath.hpp>

#include "Node.hpp"

#include <any>
#include "SelectPanel.hpp"


namespace inl::tool {


class NodeEditor {
public:
	NodeEditor(gxeng::GraphicsEngine* graphicsEngine, IGraph* graphEditor);

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

	void ErrorMessage(std::string msg);

	Vec2 ScreenToWorld(Vec2 screenPoint) const;

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
	gxeng::GraphicsEngine* m_graphicsEngine;
	IGraph* m_graphEditor;

	std::unique_ptr<gxeng::Scene> m_scene;
	std::unique_ptr<gxeng::Font> m_font;
	std::unique_ptr<gxeng::Camera2D> m_camera;

	std::unique_ptr<gxeng::OverlayEntity> m_highlight1, m_highlight2;

	std::vector<std::unique_ptr<Node>> m_nodes;
	std::unique_ptr<SelectPanel> m_selectPanel;
};


} // namespace inl::tool