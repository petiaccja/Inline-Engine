#include "PipelineEditorGraph.hpp"



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


const std::vector<INode*> PipelineEditorGraph::GetNodes() const {
	std::vector<INode*> nodes;
	for (const auto& pipelineNode : m_nodes) {
		nodes.push_back(pipelineNode.get());
	}
	return nodes;
}


const std::vector<Link>& PipelineEditorGraph::GetLinks() const {
	std::vector<inl::Link> links;

	for (const auto& pipelineNode : m_nodes) {
		
	}

	throw NotImplementedException();
}


void PipelineEditorGraph::Validate() {
	throw NotImplementedException();
}


std::string PipelineEditorGraph::SerializeJSON() {
	throw NotImplementedException();
}


void PipelineEditorGraph::LoadJSON(const std::string& description) {
	throw NotImplementedException();
}


} // namespace inl::gxeng