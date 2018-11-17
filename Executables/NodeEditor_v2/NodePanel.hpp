#pragma once

#include "ArrowControl.hpp"
#include "NodeControl.hpp"

#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Frame.hpp>

#include <memory>
#include <unordered_set>
#include <unordered_map>


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
			return std::hash<const NodeControl*>()(obj.source)
				^ (std::hash<const NodeControl*>()(obj.target) << 1)
				^ (std::hash<int>()(obj.sourcePort) << 2)
				^ (std::hash<int>()(obj.targetPort) << 3);
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

	void AddNode(std::shared_ptr<NodeControl> node);
	void RemoveNode(const NodeControl* node);

	void AddLink(const NodeControl* source, int sourcePort, const NodeControl* target, int targetPort);
	void RemoveLink(const NodeControl* source, int sourcePort, const NodeControl* target, int targetPort);

	void Clear();

	Event<Vec2> OnContextMenu;
	Event<const NodeControl*> OnDeleteNode;
	Event<const NodeControl*, int, const NodeControl*, int> OnAddLink;
	Event<const NodeControl*, int, const NodeControl*, int> OnDeleteLink;

private:
	void UpdateNodesPositions();
	void UpdateArrowPositions();
	void UpdateArrowPosition(const ArrowKey& key, ArrowControl& arrow);

	void OnNodeDragBegin(Control* control, Vec2 dragOrigin);
	void OnNodeDragged(Control* control, Vec2 dragTarget);
	void OnNodeDragEnd(Control* control, Vec2 dragEnd, Control*);

	void OnPortDragBegin(Control* control, Vec2 dragOrigin);
	void OnPortDragged(Control* control, Vec2 dragTarget);
	void OnPortDragEnd(Control* control, Vec2 dragEnd, Control*);
private:
	gui::AbsoluteLayout m_layout;

	// Node and arrows.
	std::set<std::shared_ptr<NodeControl>, NodeControlPtrLess> m_nodes;
	std::unordered_map<ArrowKey, std::shared_ptr<ArrowControl>, ArrowKeyHash> m_arrows;

	// Linking state.
	ArrowControl m_temporaryArrow;

	// Camera state.
	Vec2 m_center = { 0, 0 };
	float m_zoom = 1.0f;

	// Input actions.
	Vec2 m_dragOffset;
	NodeControl* m_draggedNode = nullptr;
	PortControl* m_draggedPort = nullptr;
};


} // namespace inl::tool