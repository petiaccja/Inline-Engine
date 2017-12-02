#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"


NodeBase* inl::gxeng::GraphicsNodeFactory::CreateNode(const std::string & name) const {
	NodeBase* node = NodeFactory::CreateNode(name);
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
