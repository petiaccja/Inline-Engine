#pragma once

#include <string>
#include <vector>


namespace inl {


class INode;


struct Link {
	INode *sourceNode, *targetNode;
	int sourcePort, targetPort;
};


class IGraph {
public:
	virtual ~IGraph() = default;

	virtual std::vector<std::string> GetNodeList() const = 0;

	virtual INode* AddNode(std::string name) = 0;
	virtual void RemoveNode(INode* node) = 0;

	virtual Link Link(INode* sourceNode, int sourcePort, INode* targetNode, int targetPort) = 0;
	virtual void Unlink(INode* targetNode, int targetPort) = 0;

	virtual const std::vector<INode*> GetNodes() const = 0;
	virtual const std::vector<inl::Link>& GetLinks() const = 0;

	virtual void Validate() = 0;
	virtual std::string SerializeJSON() = 0;
	virtual void LoadJSON(const std::string& description) = 0;
};



} // namespace inl