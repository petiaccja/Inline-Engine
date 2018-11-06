#include "NodePanel.hpp"


namespace inl::tool {



NodePanel::NodePanel() {
	m_layout.SetReferencePoint(gui::AbsoluteLayout::eRefPoint::CENTER);
	m_layout.SetYDown(false);
	SetLayout(m_layout);

	OnDrag += Delegate<void(Control*, Vec2, Vec2, Vec2)>{&NodePanel::OnNodeDragged, this};
}


void NodePanel::AddNode(std::shared_ptr<NodeControl> node) {
	auto [it, isNew] = m_nodes.insert(std::move(node));
	m_layout.AddChild(*it).SetPosition({0, 0});
}


void NodePanel::RemoveNode(const NodeControl* node) {
	auto it = m_nodes.find(node);
	[[likely]]
	if (it != m_nodes.end()) {
		m_nodes.erase(it);
	}
	else {
		throw InvalidArgumentException("Node is not shown on this panel.");
	}
}


void NodePanel::AddLink(const NodeControl* source, int sourcePort, const NodeControl* target, int targetPort) {
	ArrowKey key{ source, sourcePort, target, targetPort };
	auto [it, isNew] = m_arrows.insert({ key, std::make_shared<ArrowControl>() });
	m_layout.AddChild(it->second);
	UpdateArrowPosition(it->first, *it->second);
}


void NodePanel::RemoveLink(const NodeControl* source, int sourcePort, const NodeControl* target, int targetPort) {
	ArrowKey key{ source, sourcePort, target, targetPort };
	auto it = m_arrows.find(key);
	[[likely]]
	if (it != m_arrows.end()) {
		m_arrows.erase(it);
	}
	else {
		throw InvalidArgumentException("There is no link between specified nodes.");
	}
}


void NodePanel::UpdateNodesPositions() {
	
}


void NodePanel::UpdateArrowPositions() {
	
}

void NodePanel::UpdateArrowPosition(const ArrowKey& key, ArrowControl& arrow) {
	
}

void NodePanel::OnNodeDragged(Control* control, Vec2 controlOrigin, Vec2 dragOrigin, Vec2 dragTarget) {
	if (NodeControl* node; node = dynamic_cast<NodeControl*>(control)) {
		m_layout[node].SetPosition(controlOrigin + (dragTarget - dragOrigin) - m_layout.GetPosition());
	}
}


} // namespace inl::tool