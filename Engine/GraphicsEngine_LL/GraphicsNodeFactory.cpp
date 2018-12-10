#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"


NodeBase* inl::gxeng::GraphicsNodeFactory::CreateNode(const std::string& name) const {
	NodeBase* node = NodeFactory::CreateNode(name);
	if (node == nullptr) {
		node = NodeFactory_Singleton::GetInstance().CreateNode(name);
	}
	if (node == nullptr) {
		throw InvalidArgumentException("Node with given name not found.", name);
	}

	GraphicsNode* graphicsNode = dynamic_cast<GraphicsNode*>(node);
	if (graphicsNode != nullptr) {
		// do some special initialization for graphics nodes
		// like set graphics engine or...
		// it might not even be needed
	}

	return node;
}

std::vector<NodeFactory::NodeInfo> gxeng::GraphicsNodeFactory::EnumerateNodes() const {
	auto normalNodes = NodeFactory_Singleton::GetInstance().EnumerateNodes();
	auto myNodes = NodeFactory::EnumerateNodes();

	auto allNodes = std::move(normalNodes);
	allNodes.insert(allNodes.end(), myNodes.begin(), myNodes.end());

	return allNodes;
}
