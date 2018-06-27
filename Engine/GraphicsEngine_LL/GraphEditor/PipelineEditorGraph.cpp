#include "PipelineEditorGraph.hpp"

#include <BaseLibrary/GraphEditor/GraphParser.hpp>
#include <BaseLibrary/Range.hpp>
#include "../../../Executables/NodeEditor/Node.hpp"


namespace inl::gxeng {


PipelineEditorGraph::PipelineEditorGraph(const GraphicsNodeFactory& factory)
	: m_factory(factory)
{}


std::vector<std::string> PipelineEditorGraph::GetNodeList() const {
	auto nodeList = m_factory.EnumerateNodes();
	std::vector<std::string> nameList;

	for (auto info : nodeList) {
		nameList.push_back(info.group + "/" + info.name);
	}

	return nameList;
}


INode* PipelineEditorGraph::AddNode(std::string name) {
	std::unique_ptr<NodeBase> realNode(m_factory.CreateNode(name));
	auto node = std::make_unique<PipelineEditorNode>(std::move(realNode));
	INode* ptr = node.get();
	m_nodes.push_back(std::move(node));
	return ptr;
}


void PipelineEditorGraph::RemoveNode(INode* node) {
	// Shitty O(n) performance!

	auto it = m_nodes.begin();
	for (; it != m_nodes.end(); ++it) {
		if (static_cast<INode*>(it->get()) == node) {
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


Link PipelineEditorGraph::Link(INode* sourceNode, int sourcePort, INode* targetNode, int targetPort) {
	PipelineEditorNode* sourcePipelineNode = dynamic_cast<PipelineEditorNode*>(sourceNode);
	PipelineEditorNode* targetPipelineNode = dynamic_cast<PipelineEditorNode*>(targetNode);

	assert(sourcePort < sourcePipelineNode->GetNumOutputs());
	assert(targetPort < targetPipelineNode->GetNumInputs());

	sourcePipelineNode->GetRealNode()->GetOutput(sourcePort)->Link(targetPipelineNode->GetRealNode()->GetInput(targetPort));

	inl::Link link;
	link.sourceNode = sourceNode;
	link.sourcePort = sourcePort;
	link.targetNode = targetNode;
	link.targetPort = targetPort;
	return link;
}


void PipelineEditorGraph::Unlink(INode* targetNode, int targetPort) {
	PipelineEditorNode* targetPipelineNode = dynamic_cast<PipelineEditorNode*>(targetNode);

	assert(targetPort < targetPipelineNode->GetNumInputs());

	targetPipelineNode->GetRealNode()->GetInput(targetPort)->Unlink();
}


std::vector<INode*> PipelineEditorGraph::GetNodes() const {
	std::vector<INode*> nodes;
	for (const auto& pipelineNode : m_nodes) {
		nodes.push_back(pipelineNode.get());
	}
	return nodes;
}


std::vector<Link> PipelineEditorGraph::GetLinks() const {
	std::vector<inl::Link> links;

	struct Rec {
		PipelineEditorNode* parent;
		int index;
	};

	std::map<InputPortBase*, Rec> inputMap;
	std::map<OutputPortBase*, Rec> outputMap;

	for (const auto& node : m_nodes) {
		for (auto i : Range(node->GetNumInputs())) {
			inputMap.insert_or_assign(node->GetRealNode()->GetInput(i), Rec{ node.get(), i });
		}
		for (auto i : Range(node->GetNumOutputs())) {
			outputMap.insert_or_assign(node->GetRealNode()->GetOutput(i), Rec{ node.get(), i });
		}
	}

	for (auto& dst : inputMap) {
		InputPortBase* input = dst.first;
		OutputPortBase* link = input->GetLink();


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


void PipelineEditorGraph::Validate() {
	throw NotImplementedException();
}


std::string PipelineEditorGraph::SerializeJSON() {
	std::vector<const NodeBase*> nodes;
	std::vector<NodeMetaDescription> metaData;
	for (auto& node : m_nodes) {
		nodes.push_back(node->GetRealNode());
		metaData.push_back(node->GetMetaData());
	}

	auto FindName = [&](const NodeBase& node) {
		auto[group, className] = m_factory.GetFullName(typeid(node));
		return group + "/" + className;
	};

	return GraphParser::Serialize(nodes.data(), metaData.data(), nodes.size(), FindName);
}


void PipelineEditorGraph::LoadJSON(const std::string& description) {
	GraphParser parser;
	std::vector<std::unique_ptr<NodeBase>> nodeObjects;

	// Parse json.
	parser.Parse(description);

	// Create nodes with initial values.
	for (auto& nodeDesc : parser.GetNodes()) {
		std::unique_ptr<NodeBase> nodeObject(m_factory.CreateNode(nodeDesc.cl));
		if (nodeDesc.name) {
			nodeObject->SetDisplayName(nodeDesc.name.value());
		}

		for (int i = 0; i < nodeObject->GetNumInputs() && i < nodeDesc.defaultInputs.size(); ++i) {
			if (nodeDesc.defaultInputs[i]) {
				nodeObject->GetInput(i)->SetConvert(nodeDesc.defaultInputs[i].value());
			}
		}

		nodeObjects.push_back(std::move(nodeObject));
	}

	// Link nodes above.
	for (auto& info : parser.GetLinks()) {
		NodeBase *src = nullptr, *dst = nullptr;
		OutputPortBase* srcp = nullptr;
		InputPortBase* dstp = nullptr;

		// Find src and dst nodes.
		size_t srcNodeIdx = parser.FindNode(info.srcid, info.srcname);
		size_t dstNodeIdx = parser.FindNode(info.dstid, info.dstname);

		src = nodeObjects[srcNodeIdx].get();
		dst = nodeObjects[dstNodeIdx].get();

		// Find src and dst ports.
		srcp = parser.FindOutputPort(src, info.srcpidx, info.srcpname);
		dstp = parser.FindInputPort(dst, info.dstpidx, info.dstpname);

		// Link said ports
		srcp->Link(dstp);
	}

	// Set internal data structures.
	m_nodes.clear();
	m_nodes.reserve(nodeObjects.size());
	for (auto i : Range(nodeObjects.size())) {
		auto& realNode = nodeObjects[i];
		NodeMetaDescription metaData = parser.GetNodes()[i].metaData;
		m_nodes.emplace_back(std::make_unique<PipelineEditorNode>(std::move(realNode)));
		m_nodes.back()->SetMetaData(metaData);
	}
}


} // namespace inl::gxeng