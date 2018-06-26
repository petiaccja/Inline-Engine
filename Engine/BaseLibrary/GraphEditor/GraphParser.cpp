#include "GraphParser.hpp"

#include <rapidjson/encodings.h>
#include <rapidjson/document.h>
#include "BaseLibrary/Range.hpp"


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


InputPortBase* GraphParser::FindInputPort(NodeBase* holder, const std::optional<int>& index,
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


InputPortBase* GraphParser::FindInputPort(NodeBase* holder, const std::string& name) {
	for (auto i : Range(holder->GetNumInputs())) {
		if (holder->GetInputName(i) == name) {
			return holder->GetInput(i);
		}
	}
	throw OutOfRangeException("Port was not found.");
}


InputPortBase* GraphParser::FindInputPort(NodeBase* holder, int index) {
	if (index < holder->GetNumInputs() && 0 < index) {
		return holder->GetInput(index);
	}
	throw OutOfRangeException("Port was not found.");
}


OutputPortBase* GraphParser::FindOutputPort(NodeBase* holder, const std::optional<int>& index,
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


OutputPortBase* GraphParser::FindOutputPort(NodeBase* holder, const std::string& name) {
	for (auto i : Range(holder->GetNumOutputs())) {
		if (holder->GetOutputName(i) == name) {
			return holder->GetOutput(i);
		}
	}
	throw OutOfRangeException("Port was not found.");
}


OutputPortBase* GraphParser::FindOutputPort(NodeBase* holder, int index) {
	if (index < holder->GetNumOutputs() && 0 < index) {
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

	AssertThrow(doc.IsObject(), "JSON root must be an object with member arrays \"nodes\" and \"links\".");
	AssertThrow(doc.HasMember("nodes") && doc["nodes"].IsArray(), "JSON root must have \"nodes\" member array.");
	AssertThrow(doc.HasMember("links") && doc["links"].IsArray(), "JSON root must have \"links\" member array.");

	auto& nodes = doc["nodes"];
	auto& links = doc["links"];
	std::vector<NodeDescription> nodeCreationInfos;
	std::vector<LinkDescription> linkCreationInfos;

	for (SizeType i = 0; i < nodes.Size(); ++i) {
		NodeDescription info = ParseNode(nodes[i]);
		nodeCreationInfos.push_back(info);
	}

	for (SizeType i = 0; i < links.Size(); ++i) {
		LinkDescription info = ParseLink(links[i]);
		linkCreationInfos.push_back(info);
	}
}


void GraphParser::CreateLookupTables() {
	std::unordered_map<int, size_t> idBook;
	std::unordered_map<std::string, size_t> nameBook;
	for (size_t i = 0; i < m_nodeDescs.size(); ++i) {
		if (m_nodeDescs[i].name) {
			auto ins = nameBook.insert({ m_nodeDescs[i].name.value(), i });
			AssertThrow(ins.second == true, "Node names must be unique.");
		}
		if (m_nodeDescs[i].id) {
			auto ins = idBook.insert({ m_nodeDescs[i].id.value(), i });
			AssertThrow(ins.second == true, "Node ids must be unique.");
		}
	}
}


//------------------------------------------------------------------------------
// Helper functions.
//------------------------------------------------------------------------------

void AssertThrow(bool condition, const std::string& text) {
	if (!condition) {
		throw InvalidArgumentException(text);
	}
};


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