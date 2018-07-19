#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <functional>

#include <InlineMath.hpp>

#include "../Graph/SerializableNode.hpp"


namespace inl {


struct NodeMetaDescription {
	Vec2i placement = {0,0};
};

struct NodeDescription {
	std::optional<int> id;
	std::optional<std::string> name;
	std::string cl;
	std::vector<std::optional<std::string>> defaultInputs;
	NodeMetaDescription metaData;
};

struct LinkDescription {
	std::optional<int> srcid, dstid;
	std::optional<std::string> srcname, dstname;
	std::optional<int> srcpidx, dstpidx;
	std::optional<std::string> srcpname, dstpname;
};

struct GraphHeader {
	std::string contentType;
};


class GraphParser {
public:
	void Parse(const std::string& json);

	// TODO: make common interface for all node/port systems.
	static std::string Serialize(const ISerializableNode* const* nodes,
								 const NodeMetaDescription* metaData,
								 size_t count,
								 std::function<std::string(const ISerializableNode&)> FindName,
								 const GraphHeader& header);

	const GraphHeader& GetHeader() const;
	const std::vector<NodeDescription>& GetNodes() const;
	const std::vector<LinkDescription>& GetLinks() const;

	size_t FindNode(const std::optional<int>& id, const std::optional<std::string>& name) const;
	size_t FindNode(int id) const;
	size_t FindNode(const std::string& name) const;

	ISerializableInputPort* FindInputPort(ISerializableNode* holder, const std::optional<int>& index, const std::optional<std::string>& name);
	ISerializableInputPort* FindInputPort(ISerializableNode* holder, const std::string& name);
	ISerializableInputPort* FindInputPort(ISerializableNode* holder, int index);
	ISerializableOutputPort* FindOutputPort(ISerializableNode* holder, const std::optional<int>& index, const std::optional<std::string>& name);
	ISerializableOutputPort* FindOutputPort(ISerializableNode* holder, const std::string& name);
	ISerializableOutputPort* FindOutputPort(ISerializableNode* holder, int index);

private:
	void ParseDocument(const std::string& document);
	void CreateLookupTables();
	static std::string MakeJson(std::vector<NodeDescription> nodeDescs, std::vector<LinkDescription> linkDescs, const GraphHeader& header);

private:
	GraphHeader m_header;
	std::vector<NodeDescription> m_nodeDescs;
	std::vector<LinkDescription> m_linkDescs;

	std::unordered_map<int, size_t> m_idLookup;
	std::unordered_map<std::string, size_t> m_nameLookup;

};


} // namespace inl