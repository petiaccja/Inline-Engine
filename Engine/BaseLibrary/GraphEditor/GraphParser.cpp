#include "GraphParser.hpp"
#include "IGraphEditorNode.hpp"

#include <BaseLibrary/Range.hpp>
#include <BaseLibrary/Exception/Exception.hpp>

#include <sstream>

#include <rapidjson/encodings.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>


namespace inl {

//------------------------------------------------------------------------------
// Helper functions prototypes.
//------------------------------------------------------------------------------

static void AssertThrow(bool condition, const std::string& message);
static NodeDescription ParseNode(const rapidjson::GenericValue<rapidjson::UTF8<>>& jsonObj);
static LinkDescription ParseLink(const rapidjson::GenericValue<rapidjson::UTF8<>>& jsonObj);

struct StringErrorPosition {
	int lineNumber;
	int characterNumber;
	std::string line;
};
static StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter);




//------------------------------------------------------------------------------
// GraphParser.
//------------------------------------------------------------------------------

void GraphParser::Parse(const std::string& json) {
	ParseDocument(json);
	CreateLookupTables();
}


std::string GraphParser::Serialize(const ISerializableNode* const* nodes,
								   const NodeMetaDescription* metaData,
								   size_t count,
								   std::function<std::string(const ISerializableNode&)> FindName,
								   const GraphHeader& header)
{
	// { node -> description }
	std::unordered_map<const ISerializableNode*, NodeDescription> nodeDescLookup;
	// { output port -> { parent, index in parent }}
	std::unordered_map<const ISerializableOutputPort*, std::tuple<const ISerializableNode*, int>> outputLookup;

	// Description vector.
	std::vector<NodeDescription> nodeDescs;
	std::vector<LinkDescription> linkDescs;

	// Go over all nodes, create description and output lookup.
	for (auto i : Range(count)) {
		const ISerializableNode* node = nodes[i];

		// Create output lookup.
		for (int j = 0; j < node->GetNumOutputs(); ++j) {
			outputLookup[node->GetOutput(j)] = { node, j };
		}

		// Create preliminary description.
		NodeDescription desc;
		std::string className = FindName(*node);
		desc.cl = className;
		desc.id = (int)i;
		if (!node->GetDisplayName().empty()) {
			desc.name = node->GetDisplayName();
		}
		// Add meta to description.
		if (metaData) {
			desc.metaData = metaData[i];
		}
		else {
			memset(&desc.metaData, 0, sizeof desc.metaData);
		}

		nodeDescLookup.insert({ node, desc });
	}


	// Create link descs for all ports of all nodes.
	for (auto i : Range(count)) {
		// Destination node.
		const ISerializableNode* dst = nodes[i];

		// Iterate over all input ports.
		for (int i = 0; i < dst->GetNumInputs(); ++i) {
			const ISerializableInputPort* input = dst->GetInput(i);
			const ISerializableOutputPort* output = input->GetLink();

			// If we have an output port linked, add a link desc.
			if (output) {
				auto nit = outputLookup.find(output);
				assert(nit != outputLookup.end());
				const auto[src, srcpidx] = nit->second;

				LinkDescription desc;
				if (!src->GetDisplayName().empty()) {
					desc.srcname = src->GetDisplayName();
				}
				else {
					desc.srcid = nodeDescLookup[src].id;
				}
				if (!dst->GetDisplayName().empty()) {
					desc.dstname = dst->GetDisplayName();
				}
				else {
					desc.dstid = nodeDescLookup[dst].id;
				}

				desc.srcpidx = srcpidx;
				desc.dstpidx = i;

				linkDescs.push_back(desc);
			}
			// If no link, save the default value.
			else if (input->IsSet()) {
				try {
					std::optional<std::string> value = input->ToString();

					auto& nodeInfo = nodeDescLookup[dst];
					if (nodeInfo.defaultInputs.size() < i + 1) {
						nodeInfo.defaultInputs.resize(i + 1);
					}
					nodeInfo.defaultInputs[i] = value;
				}
				catch (...) {
					// cannot be saved.
				}
			}
		}
	}

	for (auto& v : nodeDescLookup) {
		nodeDescs.push_back(v.second);
	}

	return MakeJson(std::move(nodeDescs), std::move(linkDescs), header);
}


const GraphHeader& GraphParser::GetHeader() const {
	return m_header;
}


const std::vector<NodeDescription>& GraphParser::GetNodes() const {
	return m_nodeDescs;
}


const std::vector<LinkDescription>& GraphParser::GetLinks() const {
	return m_linkDescs;
}


size_t GraphParser::FindNode(const std::optional<int>& id, const std::optional<std::string>& name) const {
	if (id) {
		return FindNode(id.value());
	}
	if (name) {
		return FindNode(name.value());
	}
	throw InvalidArgumentException("Both ID and name cannot be nullopt.");
}


size_t GraphParser::FindNode(int id) const {
	auto it = m_idLookup.find(id);
	if (it != m_idLookup.end()) {
		return it->second;
	}
	throw OutOfRangeException("Node was not found.");
}


size_t GraphParser::FindNode(const std::string& name) const {
	auto it = m_nameLookup.find(name);
	if (it != m_nameLookup.end()) {
		return it->second;
	}
	throw OutOfRangeException("Node was not found.");
}


ISerializableInputPort* GraphParser::FindInputPort(ISerializableNode* holder, const std::optional<int>& index,
										  const std::optional<std::string>& name) 
{
	if (index) {
		return FindInputPort(holder, index.value());
	}
	if (name) {
		return FindInputPort(holder, name.value());
	}
	throw InvalidArgumentException("Both index and name cannot be nullopt.");
}


ISerializableInputPort* GraphParser::FindInputPort(ISerializableNode* holder, const std::string& name) {
	for (auto i : Range(holder->GetNumInputs())) {
		if (holder->GetInputName(i) == name) {
			return holder->GetInput(i);
		}
	}
	throw OutOfRangeException("Port was not found.");
}


ISerializableInputPort* GraphParser::FindInputPort(ISerializableNode* holder, int index) {
	if (index < holder->GetNumInputs() && 0 <= index) {
		return holder->GetInput(index);
	}
	throw OutOfRangeException("Port was not found.");
}


ISerializableOutputPort* GraphParser::FindOutputPort(ISerializableNode* holder, const std::optional<int>& index,
											const std::optional<std::string>& name) 
{
	if (index) {
		return FindOutputPort(holder, index.value());
	}
	if (name) {
		return FindOutputPort(holder, name.value());
	}
	throw InvalidArgumentException("Both index and name cannot be nullopt.");
}


ISerializableOutputPort* GraphParser::FindOutputPort(ISerializableNode* holder, const std::string& name) {
	for (auto i : Range(holder->GetNumOutputs())) {
		if (holder->GetOutputName(i) == name) {
			return holder->GetOutput(i);
		}
	}
	throw OutOfRangeException("Port was not found.");
}


ISerializableOutputPort* GraphParser::FindOutputPort(ISerializableNode* holder, int index) {
	if (index < holder->GetNumOutputs() && 0 <= index) {
		return holder->GetOutput(index);
	}
	throw OutOfRangeException("Port was not found.");
}


void GraphParser::ParseDocument(const std::string& document) {
	using namespace rapidjson;

	// Parse the JSON file.
	Document doc;
	doc.Parse(document.c_str());
	ParseErrorCode ec = doc.GetParseError();
	if (ec != ParseErrorCode::kParseErrorNone) {
		size_t errorCharacter = doc.GetErrorOffset();
		auto[lineNumber, characterNumber, line] = GetStringErrorPosition(document, errorCharacter);
		throw InvalidArgumentException("JSON descripion has syntax errors.", "Check line " + std::to_string(lineNumber) + ":" + std::to_string(characterNumber));
	}
	AssertThrow(doc.IsObject(), R"(JSON root must be an object with member arrays "nodes" and "links".)");

	// Extract header.
	GraphHeader graphHeader{};
	if (doc.HasMember("header") && doc["header"].IsObject()) {
		auto& header = doc["header"];
		AssertThrow(header.HasMember("contentType") && header["contentType"].IsString(), "Header must be a struct: { string contentType; }.");
		graphHeader.contentType = header["contentType"].GetString();
	}

	// Extract nodes and links.
	AssertThrow(doc.HasMember("nodes") && doc["nodes"].IsArray(), "JSON root must have \"nodes\" member array.");
	AssertThrow(doc.HasMember("links") && doc["links"].IsArray(), "JSON root must have \"links\" member array.");

	auto& nodes = doc["nodes"];
	auto& links = doc["links"];
	std::vector<NodeDescription> nodeDescs;
	std::vector<LinkDescription> linkDescs;

	for (SizeType i = 0; i < nodes.Size(); ++i) {
		NodeDescription info = ParseNode(nodes[i]);
		nodeDescs.push_back(info);
	}

	for (SizeType i = 0; i < links.Size(); ++i) {
		LinkDescription info = ParseLink(links[i]);
		linkDescs.push_back(info);
	}

	m_header = graphHeader;
	m_nodeDescs = nodeDescs;
	m_linkDescs = linkDescs;
}


void GraphParser::CreateLookupTables() {
	std::unordered_map<int, size_t> idLookup;
	std::unordered_map<std::string, size_t> nameLookup;
	for (size_t i = 0; i < m_nodeDescs.size(); ++i) {
		if (m_nodeDescs[i].name) {
			auto ins = nameLookup.insert({ m_nodeDescs[i].name.value(), i });
			AssertThrow(ins.second == true, "Node names must be unique.");
		}
		if (m_nodeDescs[i].id) {
			auto ins = idLookup.insert({ m_nodeDescs[i].id.value(), i });
			AssertThrow(ins.second == true, "Node ids must be unique.");
		}
	}

	m_nameLookup = std::move(nameLookup);
	m_idLookup = std::move(idLookup);
}


std::string GraphParser::MakeJson(std::vector<NodeDescription> nodeDescs, std::vector<LinkDescription> linkDescs, const GraphHeader& header) {
	using namespace rapidjson;

	std::stable_sort(nodeDescs.begin(), nodeDescs.end(), [](const NodeDescription& lhs, const NodeDescription& rhs) {
		return lhs.cl < rhs.cl;
	});
	std::stable_sort(nodeDescs.begin(), nodeDescs.end(), [](const NodeDescription& lhs, const NodeDescription& rhs) {
		return lhs.name < rhs.name;
	});

	std::stable_sort(linkDescs.begin(), linkDescs.end(), [](const LinkDescription& lhs, const LinkDescription& rhs) {
		return lhs.srcname < rhs.srcname;
	});
	std::stable_sort(linkDescs.begin(), linkDescs.end(), [](const LinkDescription& lhs, const LinkDescription& rhs) {
		return lhs.dstname < rhs.dstname;
	});

	using namespace rapidjson;
	Document doc;
	auto& alloc = doc.GetAllocator();
	doc.SetObject();

	doc.AddMember("header", kObjectType, alloc);
	doc["header"].AddMember("contentType", Value().SetString(header.contentType.c_str(), alloc), alloc);

	doc.AddMember("nodes", kArrayType, alloc);
	doc.AddMember("links", kArrayType, alloc);
	auto& docNodes = doc["nodes"];
	auto& docLinks = doc["links"];

	// Add nodes to doc.
	for (auto& node : nodeDescs) {
		Value v;
		v.SetObject();
		v.AddMember("class", Value().SetString(node.cl.c_str(), alloc), alloc);
		if (node.id) {
			v.AddMember("id", node.id.value(), alloc);
		}
		if (node.name) {
			v.AddMember("name", Value().SetString(node.name.value().c_str(), alloc), alloc);
		}
		if (!node.defaultInputs.empty()) {
			v.AddMember("inputs", kArrayType, alloc);
			for (const auto& input : node.defaultInputs) {
				if (input) {
					v["inputs"].PushBack(Value().SetString(input.value().c_str(), alloc), alloc);
				}
				else {
					v["inputs"].PushBack(Value().SetObject(), alloc);
				}
			}
		}
		std::stringstream ss;
		ss << node.metaData.placement;
		v.AddMember("meta_pos", Value().SetString(ss.str().c_str(), alloc), alloc);
		docNodes.PushBack(v, alloc);
	}

	// Add links to doc.
	auto AddLinkMember = [&alloc](Value& v, const char* member, std::optional<int> num, std::optional<std::string> str) {
		if (num) {
			v.AddMember(GenericStringRef<char>(member), num.value(), alloc);
		}
		else {
			assert(str);
			v.AddMember(GenericStringRef<char>(member), Value().SetString(str.value().c_str(), alloc), alloc);
		}
	};

	for (auto& link : linkDescs) {
		Value v;
		v.SetObject();
		AddLinkMember(v, "src", link.srcid, link.srcname);
		AddLinkMember(v, "dst", link.dstid, link.dstname);
		AddLinkMember(v, "srcp", link.srcpidx, link.srcpname);
		AddLinkMember(v, "dstp", link.dstpidx, link.dstpname);
		docLinks.PushBack(v, alloc);
	}

	// Convert JSON doc to sring
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	doc.Accept(writer);

	return buffer.GetString();
}


//------------------------------------------------------------------------------
// Helper functions.
//------------------------------------------------------------------------------

void AssertThrow(bool condition, const std::string& text) {
	if (!condition) {
		throw InvalidArgumentException(text);
	}
}


NodeDescription ParseNode(const rapidjson::GenericValue<rapidjson::UTF8<>>& obj) {
	using namespace rapidjson;

	NodeDescription info;
	if (obj.HasMember("id")) {
		AssertThrow(obj["id"].IsInt(), "Node's id member must be an integer.");
		info.id = obj["id"].GetInt();
	}
	if (obj.HasMember("name")) {
		AssertThrow(obj["name"].IsString(), "Node's name member must be a string.");
		info.name = obj["name"].GetString();
	}
	AssertThrow(info.id || info.name, "Node must have either id or name.");
	AssertThrow(obj.HasMember("class") && obj["class"].IsString(), "Node must have a class.");
	info.cl = obj["class"].GetString();

	if (obj.HasMember("inputs")) {
		AssertThrow(obj["inputs"].IsArray(), "Default inputs must be specified in an array, undefined inputs as {}.");
		auto& inputs = obj["inputs"];
		for (SizeType i = 0; i < inputs.Size(); ++i) {
			if (inputs[i].IsObject() && inputs[i].ObjectEmpty()) {
				info.defaultInputs.push_back({});
			}
			else if (inputs[i].IsString()) {
				info.defaultInputs.push_back(inputs[i].GetString());
			}
			else if (inputs[i].IsInt64()) {
				info.defaultInputs.push_back(std::to_string(inputs[i].GetInt64()));
			}
			else if (inputs[i].IsDouble()) {
				info.defaultInputs.push_back(std::to_string(inputs[i].GetDouble()));
			}
			else {
				assert(false);
			}
		}
	}

	if (obj.HasMember("meta_pos")) {
		AssertThrow(obj["meta_pos"].IsString(), "Meta pos must a string of format [123, -456].");
		std::string vecStr = obj["meta_pos"].GetString();
		const char* endPtr;
		Vec2u pos = strtovec<Vec2u>(vecStr.c_str(), &endPtr);
		info.metaData.placement = pos;
	}

	return info;
};


LinkDescription ParseLink(const rapidjson::GenericValue<rapidjson::UTF8<>>& obj) {
	LinkDescription info;

	AssertThrow(obj.HasMember("src")
				&& obj.HasMember("dst")
				&& obj.HasMember("srcp")
				&& obj.HasMember("dstp"),
				"Link must have members src, dst, srcp and dstp.");

	if (obj["src"].IsString()) {
		info.srcname = obj["src"].GetString();
	}
	else if (obj["src"].IsInt()) {
		info.srcid = obj["src"].GetInt();
	}
	else {
		AssertThrow(false, "Link src must be string (name) or int (id) of the node.");
	}

	if (obj["dst"].IsString()) {
		info.dstname = obj["dst"].GetString();
	}
	else if (obj["dst"].IsInt()) {
		info.dstid = obj["dst"].GetInt();
	}
	else {
		AssertThrow(false, "Link dst must be string (name) or int (id) of the node.");
	}

	if (obj["srcp"].IsString()) {
		info.srcpname = obj["srcp"].GetString();
	}
	else if (obj["srcp"].IsInt()) {
		info.srcpidx = obj["srcp"].GetInt();
	}
	else {
		AssertThrow(false, "Link srcp must be string (name) or int (index) of the port.");
	}

	if (obj["dstp"].IsString()) {
		info.dstpname = obj["dstp"].GetString();
	}
	else if (obj["dstp"].IsInt()) {
		info.dstpidx = obj["dstp"].GetInt();
	}
	else {
		AssertThrow(false, "Link dstp must be string (name) or int (index) of the port.");
	}

	return info;
}


StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter) {
	int currentCharacter = 0;
	int characterNumber = 0;
	int lineNumber = 0;
	while (currentCharacter < errorCharacter && currentCharacter < str.size()) {
		if (str[currentCharacter] == '\n') {
			++lineNumber;
			characterNumber = 0;
		}
		++currentCharacter;
		++characterNumber;
	}

	size_t lineBegIdx = str.rfind('\n', errorCharacter);
	size_t lineEndIdx = str.find('\n', errorCharacter);

	if (lineBegIdx = str.npos) {
		lineBegIdx = 0;
	}
	else {
		++lineBegIdx; // we don't want to keep the '\n'
	}
	if (lineEndIdx != str.npos && lineEndIdx > 0) {
		--lineEndIdx; // we don't want to keep the '\n'
	}

	return { lineNumber, characterNumber, str.substr(lineBegIdx, lineEndIdx) };
}



} // namespace inl