#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <InlineMath.hpp>

#include "../Graph/Node.hpp"
#include "GraphicsEngine_LL/GraphicsPortConverters.hpp"


namespace inl {


struct NodeDescription {
	std::optional<int> id;
	std::optional<std::string> name;
	std::string cl;
	std::vector<std::optional<std::string>> defaultInputs;
	Vec2i placement;
};

struct LinkDescription {
	std::optional<int> srcid, dstid;
	std::optional<std::string> srcname, dstname;
	std::optional<int> srcpidx, dstpidx;
	std::optional<std::string> srcpname, dstpname;
};


class GraphParser {
public:
	void Parse(const std::string& json);

	const std::vector<NodeDescription>& GetNodes() const;
	const std::vector<LinkDescription>& GetLinks() const;

	size_t FindNode(const std::optional<int>& id, const std::optional<std::string>& name) const;
	size_t FindNode(int id) const;
	size_t FindNode(const std::string& name) const;

	InputPortBase* FindInputPort(NodeBase* holder, const std::optional<int>& index, const std::optional<std::string>& name);
	InputPortBase* FindInputPort(NodeBase* holder, const std::string& name);
	InputPortBase* FindInputPort(NodeBase* holder, int index);
	OutputPortBase* FindOutputPort(NodeBase* holder, const std::optional<int>& index, const std::optional<std::string>& name);
	OutputPortBase* FindOutputPort(NodeBase* holder, const std::string& name);
	OutputPortBase* FindOutputPort(NodeBase* holder, int index);

private:
	void ParseDocument(const std::string& document);
	void CreateLookupTables();

private:
	std::vector<NodeDescription> m_nodeDescs;
	std::vector<LinkDescription> m_linkDescs;

	std::unordered_map<int, size_t> m_idLookup;
	std::unordered_map<std::string, size_t> m_nameLookup;

};


} // namespace inl