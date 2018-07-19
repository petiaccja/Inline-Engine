#pragma once

#include <string>
#include <vector>
#include "../Exception/Exception.hpp"



namespace inl {


class IGraphEditorNode;


struct Link {
	IGraphEditorNode *sourceNode, *targetNode;
	int sourcePort, targetPort;
};


class IEditorGraph {
public:
	virtual ~IEditorGraph() = default;

	virtual std::vector<std::string> GetNodeList() const = 0;
	virtual void SetNodeList(const std::vector<std::string>&) { throw InvalidCallException("This graph does not support manual node list."); }

	virtual IGraphEditorNode* AddNode(std::string name) = 0;
	virtual void RemoveNode(IGraphEditorNode* node) = 0;

	virtual Link Link(IGraphEditorNode* sourceNode, int sourcePort, IGraphEditorNode* targetNode, int targetPort) = 0;
	virtual void Unlink(IGraphEditorNode* targetNode, int targetPort) = 0;

	virtual std::vector<IGraphEditorNode*> GetNodes() const = 0;
	virtual std::vector<inl::Link> GetLinks() const = 0;

	virtual void Validate() = 0;
	virtual std::string SerializeJSON() = 0;
	virtual void LoadJSON(const std::string& description) = 0;

	virtual const std::string& GetContentType() const = 0;
	virtual void Clear() = 0;

};



} // namespace inl