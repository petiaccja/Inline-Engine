#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"


NodeBase* inl::gxeng::GraphicsNodeFactory::CreateNode(const std::string & name) {
	NodeBase* node = NodeFactory::CreateNode(name);
	if (node == nullptr) {
		return nullptr;
	}

	GraphicsNode* graphicsNode = dynamic_cast<GraphicsNode*>(node);
	if (graphicsNode != nullptr) {
		// do some special initialization for graphics nodes
		// like set graphics engine or...
		assert(false);
	}

	return node;
}
