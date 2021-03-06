#pragma once

#include "ArrowControl.hpp"
#include "NodeControl.hpp"

#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Frame.hpp>
#include <BaseLibrary/Platform/Input.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <BaseLibrary/HashCombine.hpp>


namespace inl::tool {


class NodePanel : public gui::Frame {
private:
	struct ArrowKey {
		const NodeControl* source = nullptr;
		int sourcePort = 0;
		const NodeControl* target = nullptr;
		int targetPort = 0;
		bool operator==(const ArrowKey& rhs) const {
			return source == rhs.source && target == rhs.target && sourcePort == rhs.sourcePort && targetPort == rhs.targetPort;
		}
	};
	struct ArrowKeyHash {
		size_t operator()(const ArrowKey& obj) const {
			size_t hash = std::hash<const NodeControl*>()(obj.source);
			hash = CombineHash(hash, std::hash<const NodeControl*>()(obj.target));
			hash = CombineHash(hash, std::hash<int>()(obj.sourcePort));
			hash = CombineHash(hash, std::hash<int>()(obj.targetPort));
			return hash;
		}
	};
	class NodeControlPtrLess {
	public:
		bool operator()(const std::shared_ptr<NodeControl>& lhs, const std::shared_ptr<NodeControl>& rhs) const {
			return lhs < rhs;
		}
		bool operator()(const std::shared_ptr<NodeControl>& lhs, const NodeControl* rhs) const {
			return lhs.get() < rhs;
		}
		bool operator()(const NodeControl* lhs, const std::shared_ptr<NodeControl>& rhs) const {
			return lhs < rhs.get();
		}
		using is_transparent = void*;
	};

public:
	NodePanel();

	void AddNode(std::shared_ptr<NodeControl> node, Vec2 position = {0,0});
	void RemoveNode(const NodeControl* node);

	void AddLink(const NodeControl* source, int sourcePort, const NodeControl* target, int targetPort);
	void RemoveLink(const NodeControl* source, int sourcePort, const NodeControl* target, int targetPort);

	void Clear();

	Event<Vec2> OnContextMenu;
	Event<const NodeControl*> OnDeleteNode;
	Event<const NodeControl*, int, const NodeControl*, int> OnAddLink;
	Event<const NodeControl*, int, const NodeControl*, int> OnDeleteLink;

	void UpdateStyle() override;
	void SetSize(const Vec2& size) override;
private:
	void OffsetAllNodes(Vec2 offset);
	void UpdateArrowPosition(const ArrowKey& key, ArrowControl& arrow);
	void UpdateArrowPositions();

	void OnNodeDragBegin(Control* control, Vec2 dragOrigin);
	void OnNodeDragged(Control* control, Vec2 dragTarget);
	void OnNodeDragEnd(Control* control, Vec2 dragEnd, Control*);

	void OnPortDragBegin(Control* control, Vec2 dragOrigin);
	void OnPortDragged(Control* control, Vec2 dragTarget);
	void OnPortDragEnd(Control* control, Vec2 dragEnd, Control*);

	void OnPanViewBegin(Control* control, Vec2 dragOrigin);
	void OnPanView(Control* control, Vec2 dragTarget);
	void OnZoomView(Control* control, Vec2 around, float value);

	void OnSelect(Control* control);
	void OnDeselect(Control* control);

	void SelectNode(NodeControl* node);
	void SelectPort(PortControl* port);
	void SelectArrow(ArrowControl* arrow);
	void DeselectNode();
	void DeselectPort();
	void DeselectArrow();

	void OnShortcutPressed(Control*, eKey key);

private:
	gui::AbsoluteLayout m_layout;

	// Node and arrows.
	std::set<std::shared_ptr<NodeControl>, NodeControlPtrLess> m_nodes;
	std::unordered_map<ArrowKey, std::shared_ptr<ArrowControl>, ArrowKeyHash> m_arrows;
	std::unordered_map<const ArrowControl*, ArrowKey> m_arrowKeys;

	// Linking state.
	ArrowControl m_temporaryArrow;

	// Camera state.
	Vec2 m_center = { 0, 0 };
	float m_zoom = 1.0f;

	// Input actions.
	Vec2 m_dragOrigin;
	Vec2 m_draggedNodeOrigin;
	NodeControl* m_draggedNode = nullptr;
	PortControl* m_draggedPort = nullptr;
	Vec2 m_panOrigin = { 0, 0 };

	// Selecting nodes and ports.
	NodeControl* m_selectedNode = nullptr;
	PortControl* m_selectedPort = nullptr;
	ArrowControl* m_selectedArrow = nullptr;
};


} // namespace inl::tool