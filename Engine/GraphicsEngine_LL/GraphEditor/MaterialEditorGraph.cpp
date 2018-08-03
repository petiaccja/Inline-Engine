#include "MaterialEditorGraph.hpp"
#include "../ShaderManager.hpp"
#include <BaseLibrary/Range.hpp>
#include <map>


namespace inl::gxeng {


MaterialEditorGraph::MaterialEditorGraph(const ShaderManager& shaderManager) 
	: m_shaderManager(shaderManager)
{}


std::vector<std::string> MaterialEditorGraph::GetNodeList() const {
	return m_nodeList;
}


void MaterialEditorGraph::SetNodeList(const std::vector<std::string>& nodeList) {
	// TODO: check if shader codes actually exist in shader manager.
	m_nodeList = std::move(nodeList);
}


IGraphEditorNode* MaterialEditorGraph::AddNode(std::string name) {
	auto realNode = std::make_unique<MaterialShaderEquation>(&m_shaderManager);
	realNode->SetSourceFile(name);
	auto node = std::make_unique<MaterialEditorNode>(std::move(realNode));
	m_nodes.push_back(std::move(node));
	return m_nodes.back().get();
}


void MaterialEditorGraph::RemoveNode(IGraphEditorNode* node) {
	// Shitty O(n) performance!

	auto it = m_nodes.begin();
	for (; it != m_nodes.end(); ++it) {
		if (static_cast<IGraphEditorNode*>(it->get()) == node) {
			break;
		}
	}

	if (it != m_nodes.end()) {
		m_nodes.erase(it);
	}
	else {
		throw InvalidArgumentException("Provided node is not part of this graph.");
	}
}


inl::Link MaterialEditorGraph::Link(IGraphEditorNode* sourceNode, int sourcePort, IGraphEditorNode* targetNode, int targetPort) {
	MaterialEditorNode* sourceMaterialNode = dynamic_cast<MaterialEditorNode*>(sourceNode);
	MaterialEditorNode* targetMaterialNode = dynamic_cast<MaterialEditorNode*>(targetNode);
	assert(sourceMaterialNode);
	assert(targetMaterialNode);

	assert(sourcePort < sourceMaterialNode->GetNumOutputs());
	assert(targetPort < targetMaterialNode->GetNumInputs());

	sourceMaterialNode->GetRealNode()->GetOutput(sourcePort)->Link(targetMaterialNode->GetRealNode()->GetInput(targetPort));

	inl::Link link;
	link.sourceNode = sourceNode;
	link.sourcePort = sourcePort;
	link.targetNode = targetNode;
	link.targetPort = targetPort;
	return link;
}


void MaterialEditorGraph::Unlink(IGraphEditorNode* targetNode, int targetPort) {
	MaterialEditorNode* targetMaterialNode = dynamic_cast<MaterialEditorNode*>(targetNode);

	assert(targetMaterialNode != nullptr);
	assert(targetPort < targetMaterialNode->GetNumInputs());

	targetMaterialNode->GetRealNode()->GetInput(targetPort)->Unlink();
}


std::vector<IGraphEditorNode*> MaterialEditorGraph::GetNodes() const {
	std::vector<IGraphEditorNode*> nodes;
	for (const auto& pipelineNode : m_nodes) {
		nodes.push_back(pipelineNode.get());
	}
	return nodes;
}


std::vector<inl::Link> MaterialEditorGraph::GetLinks() const {
	std::vector<inl::Link> links;

	struct Rec {
		MaterialEditorNode* parent;
		int index;
	};

	std::map<const MaterialShaderInput*, Rec> inputMap;
	std::map<const MaterialShaderOutput*, Rec> outputMap;

	for (const auto& node : m_nodes) {
		for (auto i : Range(node->GetNumInputs())) {
			inputMap.insert_or_assign(node->GetRealNode()->GetInput(i), Rec{ node.get(), i });
		}
		for (auto i : Range(node->GetNumOutputs())) {
			outputMap.insert_or_assign(node->GetRealNode()->GetOutput(i), Rec{ node.get(), i });
		}
	}

	for (auto& dst : inputMap) {
		const MaterialShaderInput* input = dst.first;
		const MaterialShaderOutput* link = input->GetLink();


		if (link) {
			auto& src = *outputMap.find(link);

			inl::Link link;
			link.sourceNode = src.second.parent;
			link.sourcePort = src.second.index;
			link.targetNode = dst.second.parent;
			link.targetPort = dst.second.index;
			links.push_back(link);
		}
	}

	return links;
}


void MaterialEditorGraph::Validate() {
	throw NotImplementedException("Fuck this method I'm lazy.");
}


std::string MaterialEditorGraph::SerializeJSON() {
	std::vector<const ISerializableNode*> nodes;
	std::vector<NodeMetaDescription> metaData;
	for (auto& node : m_nodes) {
		nodes.push_back(node->GetRealNode());
		metaData.push_back(node->GetMetaData());
	}

	auto FindName = [&](const ISerializableNode& node) {
		return node.GetClassName();
	};

	GraphHeader header;
	header.contentType = GetContentType();

	return GraphParser::Serialize(nodes.data(), metaData.data(), nodes.size(), FindName, header);
}


void MaterialEditorGraph::LoadJSON(const std::string& description) {
	GraphParser parser;
	std::vector<std::unique_ptr<MaterialShader>> nodeObjects;

	// Parse json.
	parser.Parse(description);
	if (parser.GetHeader().contentType != GetContentType()) {
		throw NotSupportedException("Graph type is not supported.");
	}

	// Create nodes with initial values.
	for (auto& nodeDesc : parser.GetNodes()) {
		auto nodeObject = std::make_unique<MaterialShaderEquation>(&m_shaderManager);
		nodeObject->SetSourceFile(nodeDesc.cl);
		if (nodeDesc.name) {
			nodeObject->SetDisplayName(nodeDesc.name.value());
		}

		nodeObjects.push_back(std::move(nodeObject));
	}

	// Link nodes above.
	for (auto& info : parser.GetLinks()) {
		MaterialShader *src = nullptr, *dst = nullptr;
		MaterialShaderOutput* srcp = nullptr;
		MaterialShaderInput* dstp = nullptr;

		// Find src and dst nodes.
		size_t srcNodeIdx = parser.FindNode(info.srcid, info.srcname);
		size_t dstNodeIdx = parser.FindNode(info.dstid, info.dstname);

		src = nodeObjects[srcNodeIdx].get();
		dst = nodeObjects[dstNodeIdx].get();

		// Find src and dst ports.
		srcp = static_cast<MaterialShaderOutput*>(parser.FindOutputPort(src, info.srcpidx, info.srcpname));
		dstp = static_cast<MaterialShaderInput*>(parser.FindInputPort(dst, info.dstpidx, info.dstpname));

		// Link said ports
		try {
			srcp->Link(dstp);
		}
		catch (Exception& ex) {
			std::stringstream ss;
			ss << " while linking " << (info.srcname ? info.srcname.value() : "");
			if (info.srcid) {
				ss << "(" << info.srcid.value() << ")";
			}
			ss << ":" << (info.srcpidx ? info.srcpidx.value() : -1);
			ss << " and " << (info.dstname ? info.dstname.value() : "");
			if (info.dstid) {
				ss << "(" << info.dstid.value() << ")";
			}
			ss << ":" << (info.dstpidx ? info.dstpidx.value() : -1);
			throw InvalidArgumentException(ex.Message() + ss.str(), ex.Subject());
		}
	}

	// Set internal data structures.
	m_nodes.clear();
	m_nodes.reserve(nodeObjects.size());
	for (auto i : Range(nodeObjects.size())) {
		auto& realNode = nodeObjects[i];
		NodeMetaDescription metaData = parser.GetNodes()[i].metaData;
		m_nodes.emplace_back(std::make_unique<MaterialEditorNode>(std::move(realNode)));
		m_nodes.back()->SetMetaData(metaData);
	}
}


const std::string& MaterialEditorGraph::GetContentType() const {
	static const std::string contentType = "material";
	return contentType;
}


void MaterialEditorGraph::Clear() {
	m_nodes.clear();
}




} // namespace inl::gxeng