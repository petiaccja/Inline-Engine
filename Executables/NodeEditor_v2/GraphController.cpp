#include "GraphController.hpp"
#include <BaseLibrary/Delegate.hpp>



namespace inl::tool {


void GraphController::SetNodePanel(NodePanel& panel) {
	if (m_view) {
		UnregisterView(m_view);
	}

	// TODO: don't clear but rebuild
	Clear();
	m_view = &panel;
	RegisterView(m_view);
}


void GraphController::SetEditorGraph(IEditorGraph& editorGraph) {
	// TODO: don't clear but rebuild
	Clear();
	m_model = &editorGraph;
}


void GraphController::Clear() {
	if (m_view) {
		m_view->Clear();
	}
	if (m_model) {
		m_model->Clear();
	}
}

void GraphController::OnLink(const NodeControl* output, int outPort, const NodeControl* input, int inPort) {
	m_view->AddLink(output, outPort, input, inPort);
}


void GraphController::RegisterView(NodePanel* view) {
	m_view->OnAddLink += Delegate<void(const NodeControl*, int, const NodeControl*, int)>{&GraphController::OnLink, this};
}


void GraphController::UnregisterView(NodePanel* view) {
	m_view->OnAddLink -= Delegate<void(const NodeControl*, int, const NodeControl*, int)>{&GraphController::OnLink, this};
}


}
