#include "GraphController.hpp"

#include <BaseLibrary/Delegate.hpp>
#include <BaseLibrary/Range.hpp>
#include <BaseLibrary/StringUtil.hpp>

#include <regex>


namespace inl::tool {


std::string TidyTypename(std::string name) {
	static std::regex ns1("inl::", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex ns2("gxeng::", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex ns3("nodes::", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex cl("class\\s*", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex st("struct\\s*", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex cnst("const", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	static std::regex ptr64("__ptr64", std::regex_constants::ECMAScript | std::regex_constants::optimize);

	name = std::regex_replace(name, ns1, "");
	name = std::regex_replace(name, ns2, "");
	name = std::regex_replace(name, ns3, "");
	name = std::regex_replace(name, cl, "");
	name = std::regex_replace(name, st, "");
	name = std::regex_replace(name, cnst, "");
	name = std::regex_replace(name, ptr64, "");

	size_t bs = name.find("basic_string");
	if (bs != name.npos) {
		int counter = 0;
		bool flag = false;
		auto it = name.begin() + bs;
		while ((!flag || counter > 0) && it != name.end()) {
			counter += *it == '>' ? -1 : (*it == '<' ? +1 : 0);
			flag = flag || (*it == '<');
			++it;
		}
		name.erase(name.begin() + bs, it);
		name.insert(bs, "string");
	}

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


void GraphController::Load(const std::string& desc, const std::vector<IEditorGraph*>& editors) {
	// Try to load the file with one of the editor interfaces.
	bool loaded = false;
	for (auto& graphEditor : editors) {
		try {
			graphEditor->LoadJSON(desc);
			m_model = graphEditor;
			loaded = true;
			break;
		}
		catch (NotSupportedException&) {
			// Graph editor does not support specified type.
		}
	}
	if (!loaded) {
		throw NotSupportedException("Graph type is not supported.");
	}

	m_nodes.clear();
	m_view->Clear();

	// Set node list.
	std::vector<std::u32string> nameList;
	for (const auto& name : m_model->GetNodeList()) {
		nameList.push_back(EncodeString<char32_t>(name));
	}
	m_selectPanel->SetChoices(nameList);

	// Create nodes.
	std::map<IGraphEditorNode*, NodeControl*> inversionMap;

	auto graphNodes = m_model->GetNodes();
	for (auto realNode : graphNodes) {
		auto viewNode = CreateViewNode(realNode);
		Vec2 position = realNode->GetMetaData().placement;
		m_view->AddNode(viewNode, position);
		inversionMap[realNode] = viewNode.get();
		m_nodes.insert({ viewNode, realNode });
	}

	auto graphLinks = m_model->GetLinks();
	for (auto realLink : graphLinks) {
		NodeControl* src = inversionMap[realLink.sourceNode];
		NodeControl* tar = inversionMap[realLink.targetNode];

		m_view->AddLink(src, realLink.sourcePort, tar, realLink.targetPort);
	}
}


std::string GraphController::Serialize() const {
	Vec2 averagePosition = { 0, 0 };
	for (const auto& node : m_nodes) {
		averagePosition += node.first->GetPosition();
	}
	averagePosition /= (float)m_nodes.size();

	for (const auto& node : m_nodes) {
		NodeMetaDescription metaData = node.second->GetMetaData();
		metaData.placement = node.first->GetPosition() - averagePosition;
		node.second->SetMetaData(metaData);
	}

	return m_model->SerializeJSON();
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


std::shared_ptr<NodeControl> GraphController::CreateViewNode(IGraphEditorNode* newNode) const {
	auto viewNode = std::make_shared<NodeControl>();

	std::vector<std::pair<std::string, std::string>> inputNames, outputNames;

	for (const auto inputIdx : Range(newNode->GetNumInputs())) {
		inputNames.push_back({ newNode->GetInputName(inputIdx), TidyTypename(newNode->GetInputTypeName(inputIdx)) });
	}
	for (const auto outputIdx : Range(newNode->GetNumOutputs())) {
		outputNames.push_back({ newNode->GetOutputName(outputIdx), TidyTypename(newNode->GetOutputTypeName(outputIdx)) });
	}

	viewNode->SetSize({ 300, 100 });
	viewNode->SetInputPorts(inputNames);
	viewNode->SetOutputPorts(outputNames);
	viewNode->SetName("");
	viewNode->SetType(TidyTypename(newNode->GetClassName()));

	return viewNode;
}

void GraphController::OnAddNode(std::u32string name) {
	try {
		IGraphEditorNode* newNode(m_model->AddNode(EncodeString<char>(name)));
		auto newViewNode = CreateViewNode(newNode);

		m_nodes.insert({ newViewNode, newNode });
		m_view->AddNode(newViewNode);
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
	m_nodes.erase(it);
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
