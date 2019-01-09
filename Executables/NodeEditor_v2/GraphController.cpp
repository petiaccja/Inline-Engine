#include "GraphController.hpp"

#include <BaseLibrary/Delegate.hpp>
#include <BaseLibrary/Range.hpp>


namespace inl::tool {


std::string TidyTypename(std::string name) {
	static std::regex ns1("inl::", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex ns2("gxeng::", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex ns3("nodes::", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex cl("class\\s*", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex st("struct\\s*", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex cnst("const", std::regex_constants::ECMAScript | std::regex_constants::optimize);

	name = std::regex_replace(name, ns1, "");
	name = std::regex_replace(name, ns2, "");
	name = std::regex_replace(name, ns3, "");
	name = std::regex_replace(name, cl, "");
	name = std::regex_replace(name, st, "");
	name = std::regex_replace(name, cnst, "!C");

	return name;
}


void GraphController::SetSelectPanel(NodeSelectPanel& selectPanel) {
	if (m_selectPanel) {
		m_selectPanel->OnAddNode -= Delegate<void(std::u32string)>(&GraphController::OnAddNode, this);
	}
	m_selectPanel = &selectPanel;
	m_selectPanel->OnAddNode += Delegate<void(std::u32string)>(&GraphController::OnAddNode, this);
}


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
	m_nodes.clear();
}


void GraphController::OnAddNode(std::u32string name) {
	try {
		IGraphEditorNode* newNode(m_model->AddNode(EncodeString<char>(name)));
		std::shared_ptr<NodeControl> newNodeView = std::make_shared<NodeControl>();

		std::vector<std::pair<std::string, std::string>> inputNames, outputNames;

		for (const auto inputIdx : Range(newNode->GetNumInputs())) {
			inputNames.push_back({ newNode->GetInputName(inputIdx), TidyTypename(newNode->GetInputTypeName(inputIdx)) });
		}
		for (const auto outputIdx : Range(newNode->GetNumOutputs())) {
			outputNames.push_back({ newNode->GetOutputName(outputIdx), TidyTypename(newNode->GetOutputTypeName(outputIdx)) });
		}

		newNodeView->SetSize({ 300, 100 });
		newNodeView->SetInputPorts(inputNames);
		newNodeView->SetOutputPorts(outputNames);
		newNodeView->SetName("");
		newNodeView->SetType(TidyTypename(newNode->GetClassName()));

		m_nodes.insert({ newNodeView, newNode });
		m_view->AddNode(newNodeView);
	}
	catch (...) {
		std::cout << "Failed to add new node." << std::endl;
	}
}


void GraphController::OnDeleteNode(const NodeControl* node) {
	auto it = m_nodes.find(node);
	assert(it != m_nodes.end()); // View had nodes the model does not, should never get out of sync.
	m_model->RemoveNode(it->second);
	m_view->RemoveNode(node);
}


void GraphController::OnLink(const NodeControl* output, int outPort, const NodeControl* input, int inPort) {
	auto it = m_nodes.find(output);
	assert(it != m_nodes.end());
	IGraphEditorNode* outputModelNode = it->second;
	it = m_nodes.find(input);
	assert(it != m_nodes.end());
	IGraphEditorNode* inputModelNode = it->second;

	try {
		m_model->Link(outputModelNode, outPort, inputModelNode, inPort);
		m_view->AddLink(output, outPort, input, inPort);
	}
	catch (Exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}

void GraphController::OnDeleteLink(const NodeControl* output, int outPort, const NodeControl* input, int inPort) {
	IGraphEditorNode* tarNode = m_nodes.find(input)->second;

	m_model->Unlink(tarNode, inPort);
	m_view->RemoveLink(output, outPort, input, inPort);
}


void GraphController::RegisterView(NodePanel* view) {
	m_view->OnAddLink += Delegate<void(const NodeControl*, int, const NodeControl*, int)>{ &GraphController::OnLink, this };
	m_view->OnDeleteLink += Delegate<void(const NodeControl*, int, const NodeControl*, int)>{ &GraphController::OnDeleteLink, this };
	m_view->OnDeleteNode += Delegate<void(const NodeControl*)>{ &GraphController::OnDeleteNode, this };
}


void GraphController::UnregisterView(NodePanel* view) {
	m_view->OnAddLink -= Delegate<void(const NodeControl*, int, const NodeControl*, int)>{ &GraphController::OnLink, this };
}


} // namespace inl::tool
