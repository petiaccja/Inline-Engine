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
		std::string name = info.group;
		if (!name.empty()) {
			name += "/";
		}
		name += info.name;
		nameList.push_back(name);
	}

	return nameList;
}


IGraphEditorNode* PipelineEditorGraph::AddNode(std::string name) {
	std::unique_ptr<NodeBase> realNode(m_factory.CreateNode(name));
	auto node = std::make_unique<PipelineEditorNode>(std::move(realNode));
	IGraphEditorNode* ptr = node.get();
	m_nodes.push_back(std::move(node));
	return ptr;
}


void PipelineEditorGraph::RemoveNode(IGraphEditorNode* node) {
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


Link PipelineEditorGraph::Link(IGraphEditorNode* sourceNode, int sourcePort, IGraphEditorNode* targetNode, int targetPort) {
	PipelineEditorNode* sourcePipelineNode = dynamic_cast<PipelineEditorNode*>(sourceNode);
	PipelineEditorNode* targetPipelineNode = dynamic_cast<PipelineEditorNode*>(targetNode);
	assert(sourcePipelineNode);
	assert(targetPipelineNode);

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


void PipelineEditorGraph::Unlink(IGraphEditorNode* targetNode, int targetPort) {
	PipelineEditorNode* targetPipelineNode = dynamic_cast<PipelineEditorNode*>(targetNode);

	assert(targetPort < targetPipelineNode->GetNumInputs());

	targetPipelineNode->GetRealNode()->GetInput(targetPort)->Unlink();
}


std::vector<IGraphEditorNode*> PipelineEditorGraph::GetNodes() const {
	std::vector<IGraphEditorNode*> nodes;
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

	std::map<const InputPortBase*, Rec> inputMap;
	std::map<const OutputPortBase*, Rec> outputMap;

	for (const auto& node : m_nodes) {
		for (auto i : Range(node->GetNumInputs())) {
			inputMap.insert_or_assign(node->GetRealNode()->GetInput(i), Rec{ node.get(), i });
		}
		for (auto i : Range(node->GetNumOutputs())) {
			outputMap.insert_or_assign(node->GetRealNode()->GetOutput(i), Rec{ node.get(), i });
		}
	}

	for (auto& dst : inputMap) {
		const InputPortBase* input = dst.first;
		const OutputPortBase* link = input->GetLink();


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
	std::vector<const ISerializableNode*> nodes;
	std::vector<NodeMetaDescription> metaData;
	for (auto& node : m_nodes) {
		nodes.push_back(node->GetRealNode());
		metaData.push_back(node->GetMetaData());
	}

	auto FindName = [&](const ISerializableNode& node) {
		auto[group, className] = m_factory.GetFullName(typeid(node));
		return group + "/" + className;
	};

	GraphHeader header;
	header.contentType = GetContentType();

	return GraphParser::Serialize(nodes.data(), metaData.data(), nodes.size(), FindName, header);
}


void PipelineEditorGraph::LoadJSON(const std::string& description) {
	GraphParser parser;
	std::vector<std::unique_ptr<NodeBase>> nodeObjects;

	// Parse json.
	parser.Parse(description);
	if (parser.GetHeader().contentType != GetContentType()) {
		throw NotSupportedException("Graph type is not supported.");
	}

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
		srcp = static_cast<OutputPortBase*>(parser.FindOutputPort(src, info.srcpidx, info.srcpname));
		dstp = static_cast<InputPortBase*>(parser.FindInputPort(dst, info.dstpidx, info.dstpname));

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
		m_nodes.emplace_back(std::make_unique<PipelineEditorNode>(std::move(realNode)));
		m_nodes.back()->SetMetaData(metaData);
	}
}


const std::string& PipelineEditorGraph::GetContentType() const {
	static const std::string contentType = "pipeline";
	return contentType;
}


void PipelineEditorGraph::Clear() {
	m_nodes.clear();
}


} // namespace inl::gxeng