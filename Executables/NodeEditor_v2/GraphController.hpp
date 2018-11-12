#pragma once


#include <BaseLibrary/GraphEditor/IEditorGraph.hpp>
#include "NodePanel.hpp"


namespace inl::tool {


class GraphController {
public:
	GraphController() = default;

	void SetNodePanel(NodePanel& panel);
	void SetEditorGraph(IEditorGraph& editorGraph);

	void Clear();

private:
	void OnLink(const NodeControl* output, int outPort, const NodeControl* input, int inPort);
	void RegisterView(NodePanel* view);
	void UnregisterView(NodePanel* view);

private:
	NodePanel* m_view = nullptr;
	IEditorGraph* m_model = nullptr;
};


}
