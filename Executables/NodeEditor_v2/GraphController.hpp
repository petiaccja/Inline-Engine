#pragma once


#include "NodePanel.hpp"
#include "NodeSelectPanel.hpp"

#include <BaseLibrary/GraphEditor/IEditorGraph.hpp>
#include <BaseLibrary/GraphEditor/IGraphEditorNode.hpp>


namespace inl::tool {


namespace impl {

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

} // namespace impl


class GraphController {
public:
	GraphController() = default;

	void SetSelectPanel(NodeSelectPanel& selectPanel);
	void SetNodePanel(NodePanel& panel);
	void SetEditorGraph(IEditorGraph& editorGraph);

	void Clear();

private:
	void OnAddNode(std::u32string name);
	void OnDeleteNode(const NodeControl* node);
	void OnLink(const NodeControl* output, int outPort, const NodeControl* input, int inPort);
	void OnDeleteLink(const NodeControl* output, int outPort, const NodeControl* input, int inPort);
	void RegisterView(NodePanel* view);
	void UnregisterView(NodePanel* view);

private:
	NodeSelectPanel* m_selectPanel = nullptr;
	NodePanel* m_view = nullptr;
	IEditorGraph* m_model = nullptr;
	std::map<std::shared_ptr<NodeControl>, IGraphEditorNode*, impl::NodeControlPtrLess> m_nodes;
};


} // namespace inl::tool
