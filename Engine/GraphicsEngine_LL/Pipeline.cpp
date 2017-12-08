#undef IN
#undef OUT

#include "Pipeline.hpp"
#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <cassert>
#include <optional>
#include <typeinfo>


namespace inl {
namespace gxeng {


//------------------------------------------------------------------------------
// Helper structures and functions
//------------------------------------------------------------------------------

struct NodeCreationInfo {
	std::optional<int> id;
	std::optional<std::string> name;
	std::string cl;
	std::vector<std::optional<std::string>> inputs;
};

struct LinkCreationInfo {
	std::optional<int> srcid, dstid;
	std::optional<std::string> srcname, dstname;
	std::optional<int> srcpidx, dstpidx;
	std::optional<std::string> srcpname, dstpname;
};

struct StringErrorPosition {
	int lineNumber;
	int characterNumber;
	std::string line;
};


static void AssertThrow(bool condition, const std::string& message);
static NodeCreationInfo ParseNode(const rapidjson::GenericValue<rapidjson::UTF8<>>& jsonObj);
static LinkCreationInfo ParseLink(const rapidjson::GenericValue<rapidjson::UTF8<>>& jsonObj);
static StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter);

static std::string SerializeNodesAndLinks(std::vector<NodeCreationInfo> nodes, std::vector<LinkCreationInfo> links);



//------------------------------------------------------------------------------
// Iterator
//------------------------------------------------------------------------------

Pipeline::NodeIterator::NodeIterator(const Pipeline* parent, lemon::ListDigraph::NodeIt graphIt)
	: m_graphIt(graphIt), m_parent(parent)
{}

Pipeline::NodeIterator::NodeIterator()
	: m_graphIt(lemon::INVALID), m_parent(nullptr)
{}

NodeBase& Pipeline::NodeIterator::operator*() const {
	return *(operator->());
}

NodeBase* Pipeline::NodeIterator::operator->() const {
	assert(m_parent != nullptr);
	assert(m_parent->m_dependencyGraph.valid(m_graphIt));
	return (m_parent->m_nodeMap[m_graphIt]).get();
}


bool Pipeline::NodeIterator::operator==(const NodeIterator& rhs) const {
	return (m_parent == rhs.m_parent && m_graphIt == rhs.m_graphIt) || (m_parent == nullptr && rhs.m_parent == nullptr);
}

bool Pipeline::NodeIterator::operator!=(const NodeIterator& rhs) const {
	return !(*this == rhs);
}


Pipeline::NodeIterator& Pipeline::NodeIterator::operator++() {
	++m_graphIt;
	return *this;
}

Pipeline::NodeIterator Pipeline::NodeIterator::operator++(int) const {
	return NodeIterator(m_parent, ++lemon::ListDigraph::NodeIt(m_graphIt));
}


//------------------------------------------------------------------------------
// Pipeline
//------------------------------------------------------------------------------


Pipeline::Pipeline()
	: m_nodeMap(m_dependencyGraph),
	m_taskFunctionMap(m_taskGraph),
	m_taskParentMap(m_taskGraph, lemon::INVALID)
{}


Pipeline::Pipeline(Pipeline&& rhs) : Pipeline() {
	// copy graphs and maps
	lemon::DigraphCopy<decltype(rhs.m_dependencyGraph), decltype(this->m_dependencyGraph)>
		depCopy(rhs.m_dependencyGraph, this->m_dependencyGraph);
	depCopy.nodeMap(rhs.m_nodeMap, this->m_nodeMap);
	depCopy.run();

	lemon::DigraphCopy<decltype(rhs.m_taskGraph), decltype(this->m_taskGraph)>
		taskCopy(rhs.m_taskGraph, this->m_taskGraph);
	taskCopy.nodeMap(rhs.m_taskFunctionMap, this->m_taskFunctionMap);
	taskCopy.nodeMap(rhs.m_taskParentMap, this->m_taskParentMap);
	taskCopy.run();

	// clear rhs's stuff
	rhs.m_dependencyGraph.clear();
	rhs.m_taskGraph.clear();
}


Pipeline& Pipeline::operator=(Pipeline&& rhs) {
	// clean previous stuff
	Clear();
	// user ctor to create new object over this
	new (this) Pipeline(std::forward<Pipeline>(rhs));

	return *this;
}


Pipeline::~Pipeline() {
	Clear();
}



void Pipeline::CreateFromDescription(const std::string& jsonDescription, GraphicsNodeFactory& factory) {
	using namespace rapidjson;

	// Parse the JSON file.
	Document doc;
	doc.Parse(jsonDescription.c_str());
	ParseErrorCode ec = doc.GetParseError();
	if (ec != ParseErrorCode::kParseErrorNone) {
		size_t errorCharacter = doc.GetErrorOffset();
		auto [lineNumber, characterNumber, line] = GetStringErrorPosition(jsonDescription, errorCharacter);
		throw InvalidArgumentException("JSON descripion has syntax errors.", "Check line " + std::to_string(lineNumber) + ":" + std::to_string(characterNumber));
	}

	AssertThrow(doc.IsObject(), "JSON root must be an object with member arrays \"nodes\" and \"links\".");
	AssertThrow(doc.HasMember("nodes") && doc["nodes"].IsArray(), "JSON root must have \"nodes\" member array.");
	AssertThrow(doc.HasMember("links") && doc["links"].IsArray(), "JSON root must have \"links\" member array.");

	auto& nodes = doc["nodes"];
	auto& links = doc["links"];
	std::vector<NodeCreationInfo> nodeCreationInfos;
	std::vector<LinkCreationInfo> linkCreationInfos;

	for (SizeType i = 0; i < nodes.Size(); ++i) {
		NodeCreationInfo info = ParseNode(nodes[i]);
		nodeCreationInfos.push_back(info);
	}

	for (SizeType i = 0; i < links.Size(); ++i) {
		LinkCreationInfo info = ParseLink(links[i]);
		linkCreationInfos.push_back(info);
	}

	// Create lookup dictionary of nodes by name and by id.
	// {name/id of node, index of node in vector}
	std::unordered_map<int, size_t> idBook;
	std::unordered_map<std::string, size_t> nameBook;
	for (size_t i = 0; i < nodeCreationInfos.size(); ++i) {
		if (nodeCreationInfos[i].name) {
			auto ins = nameBook.insert({ nodeCreationInfos[i].name.value(), i });
			AssertThrow(ins.second == true, "Node names must be unique.");
		}
		if (nodeCreationInfos[i].id) {
			auto ins = idBook.insert({ nodeCreationInfos[i].id.value(), i });
			AssertThrow(ins.second == true, "Node ids must be unique.");
		}
	}

	// Create nodes with initial values.
	std::vector<std::shared_ptr<NodeBase>> nodeObjects;
	for (auto& info : nodeCreationInfos) {
		std::shared_ptr<NodeBase> nodeObject(factory.CreateNode(info.cl));
		if (info.name) {
			nodeObject->SetDisplayName(info.name.value());
		}

		for (int i = 0; i < nodeObject->GetNumInputs() && i < info.inputs.size(); ++i) {
			if (info.inputs[i]) {
				nodeObject->GetInput(i)->SetConvert(info.inputs[i].value());
			}
		}

		nodeObjects.push_back(std::move(nodeObject));
	}

	// Link nodes above.
	for (auto& info : linkCreationInfos) {
		NodeBase *src, *dst;
		OutputPortBase* srcp = nullptr;
		InputPortBase* dstp = nullptr;

		// Find src and dst nodes
		if (info.srcname) {
			auto it = nameBook.find(info.srcname.value());
			AssertThrow(it != nameBook.end(), "Node requested to link named " + info.srcname.value() + " not found.");
			src = nodeObjects[it->second].get();
		}
		else {
			auto it = idBook.find(info.srcid.value());
			AssertThrow(it != idBook.end(), "Node requested to link id=" + std::to_string(info.srcid.value()) + " not found.");
			src = nodeObjects[it->second].get();
		}
		if (info.dstname) {
			auto it = nameBook.find(info.dstname.value());
			AssertThrow(it != nameBook.end(), "Node requested to link named " + info.dstname.value() + " not found.");
			dst = nodeObjects[it->second].get();
		}
		else {
			auto it = idBook.find(info.dstid.value());
			AssertThrow(it != idBook.end(), "Node requested to link id=" + std::to_string(info.dstid.value()) + " not found.");
			dst = nodeObjects[it->second].get();
		}
		// Find src and dst ports
		if (info.srcpname) {
			for (int i = 0; i < src->GetNumOutputs(); ++i) {
				if (info.srcpname.value() == src->GetOutputName(i)) {
					srcp = src->GetOutput(i);
					break;
				}
			}
		}
		else {
			srcp = src->GetOutput(info.srcpidx.value());
		}
		if (info.dstpname) {
			for (int i = 0; i < dst->GetNumInputs(); ++i) {
				if (info.dstpname.value() == dst->GetInputName(i)) {
					dstp = dst->GetInput(i);
					break;
				}
			}
		}
		else {
			dstp = dst->GetInput(info.dstpidx.value());
		}
		// Link said ports
		bool linked = srcp->Link(dstp);
		AssertThrow(linked, "Ports not compatible.");
	}


	// Finish by creating the actual pipeline.
	EngineContext engineContext(1, 1);
	for (auto& node : nodeObjects) {
		if (auto graphicsNode = dynamic_cast<GraphicsNode*>(node.get())) {
			graphicsNode->Initialize(engineContext);
		}
	}

	CreateFromNodesList(nodeObjects);
}


void Pipeline::CreateFromNodesList(const std::vector<std::shared_ptr<NodeBase>> nodes) {
	// assign pipeline nodes to graph nodes
	for (auto pipelineNode : nodes) {
		lemon::ListDigraph::Node graphNode = m_dependencyGraph.addNode();
		m_nodeMap[graphNode] = pipelineNode;
	}

	// calculate graphs
	try {
		CalculateDependencyGraph();
		CalculateTaskGraph();
	}
	catch (...) {
		Clear();
		throw;
	}

	// check if graphs are DAGs
	// note: if task graph is a DAG => dep. graph must be a DAG
	bool isTaskGraphDAG = lemon::dag(m_taskGraph);
	if (!isTaskGraphDAG) {
		throw InvalidArgumentException("Supplied nodes do not make a directed acyclic graph.");
	}
}


std::string Pipeline::SerializeToJSON(const NodeFactory& factory) const {
	using namespace rapidjson;

	std::unordered_map<OutputPortBase*, std::tuple<NodeBase*, int>> outputLookup;
	std::unordered_map<NodeBase*, NodeCreationInfo> nodeCreation;
	std::vector<LinkCreationInfo> linkCreation;
	int nodeId = 0;

	for (lemon::ListDigraph::NodeIt it(m_dependencyGraph); it != lemon::INVALID; ++it) {
		NodeBase* node = m_nodeMap[it].get();
		for (int i = 0; i < node->GetNumOutputs(); ++i) {
			outputLookup[node->GetOutput(i)] = { node, i };
		}

		NodeCreationInfo info;
		auto [group, className] = factory.GetFullName(typeid(*node));
		info.cl = group + "/" + className;
		info.id = nodeId++;
		if (!node->GetDisplayName().empty()) {
			info.name = node->GetDisplayName();
		}
		nodeCreation.insert({ node, info });
	}

	for (lemon::ListDigraph::NodeIt it(m_dependencyGraph); it != lemon::INVALID; ++it) {
		NodeBase* dst = m_nodeMap[it].get();
		for (int i = 0; i < dst->GetNumInputs(); ++i) {
			InputPortBase* input = dst->GetInput(i);
			OutputPortBase* output = input->GetLink();
			if (output != nullptr) {
				auto nit = outputLookup.find(output);
				assert(nit != outputLookup.end());
				const auto [ src, srcpidx ] = nit->second;

				LinkCreationInfo info;
				if (!src->GetDisplayName().empty()) {
					info.srcname = src->GetDisplayName();
				}
				else {
					info.srcid = nodeCreation[src].id;
				}
				if (!dst->GetDisplayName().empty()) {
					info.dstname = dst->GetDisplayName();
				}
				else {
					info.dstid = nodeCreation[dst].id;
				}

				info.srcpidx = srcpidx;
				info.dstpidx = i;

				linkCreation.push_back(info);
			}
			else {
				// save default input value
				try {
					std::optional<std::string> value = input->ToString();

					auto& nodeInfo = nodeCreation[dst];
					if (nodeInfo.inputs.size() < i + 1) {
						nodeInfo.inputs.resize(i + 1);
					}
					nodeInfo.inputs[i] = value;
				}
				catch (InvalidCallException&) {
					std::cout << "Conversion failed for " << input->GetType().name() << std::endl;
				}
			}
		}
	}

	std::vector<NodeCreationInfo> nodeCreationVec;
	for (auto& v : nodeCreation) {
		nodeCreationVec.push_back(v.second);
	}

	return SerializeNodesAndLinks(nodeCreationVec, linkCreation);
}


void Pipeline::Clear() {
	for (lemon::ListDigraph::NodeIt graphNode(m_dependencyGraph); graphNode != lemon::INVALID; ++graphNode) {
		m_nodeMap[graphNode] = nullptr; // not necessary, but better make sure
	}
	m_dependencyGraph.clear();
	m_taskGraph.clear();
}


Pipeline::NodeIterator Pipeline::begin() {
	return{ this, lemon::ListDigraph::NodeIt(m_dependencyGraph) };
}


Pipeline::NodeIterator Pipeline::end() {
	return{ this, lemon::INVALID };
}


Pipeline::ConstNodeIterator Pipeline::begin() const {
	return{ this, lemon::ListDigraph::NodeIt(m_dependencyGraph) };
}


Pipeline::ConstNodeIterator Pipeline::end() const {
	return{ this, lemon::INVALID };
}



void Pipeline::CalculateTaskGraph() {
	// This structure is used to assign 
	//		{source,sink} pairs in m_taskGraph
	//		to
	//		nodes in m_dependencyGraph
	struct SourceSinkMapping {
		lemon::ListDigraph::Node source;
		lemon::ListDigraph::Node sink;
	};

	// Reset task graph completely
	m_taskGraph.clear();
	m_taskWrappers.clear();

	lemon::ListDigraph::NodeMap<SourceSinkMapping> sourceSinkMapping(m_dependencyGraph); // map targets are in m_taskGraph

	// Iterate over each node in dependency graph
	for (lemon::ListDigraph::NodeIt depNode(m_dependencyGraph); depNode != lemon::INVALID; ++depNode) {
		// Get pipeline node of this graph node
		NodeBase* pipelineNode = m_nodeMap[depNode].get();
		assert(pipelineNode != nullptr); // each graph node must have a pipeline node assigned

		// Merge subgraph of graphics pipeline node into expanded task graph
		if (gxeng::GraphicsNode* graphicsPipelineNode = dynamic_cast<gxeng::GraphicsNode*>(pipelineNode)) {
			const lemon::ListDigraph& subtaskNodes = graphicsPipelineNode->GetTaskGraph();
			const lemon::ListDigraph::NodeMap<GraphicsTask*>& subtaskMap = graphicsPipelineNode->GetTaskGraphMapping();

			// Merge-copy Task's graph into m_taskGraph, create mapping of nodes
			// Copy nodes
			int numSubtasks = lemon::countNodes(subtaskNodes);
			if (numSubtasks == 0) {
				throw InvalidArgumentException("Task has zero subtasks.");
			}
			lemon::ListDigraph::NodeMap<decltype(m_taskGraph)::Node> taskNodesToTaskGraphNodes(subtaskNodes); // maps Task's nodes to m_taskGraph's nodes
			for (lemon::ListDigraph::NodeIt taskNode(subtaskNodes); taskNode != lemon::INVALID; ++taskNode) {
				auto taskGraphNode = m_taskGraph.addNode(); // add new node
				taskNodesToTaskGraphNodes[taskNode] = taskGraphNode; // map new node to task.m_nodes' corresponding node

				m_taskFunctionMap[taskGraphNode] = subtaskMap[taskNode]; // assign subtask
				m_taskParentMap[taskGraphNode] = depNode; // assign parent of subtask
			}
			// Copy arcs
			for (lemon::ListDigraph::ArcIt arc(subtaskNodes); arc != lemon::INVALID; ++arc) {
				m_taskGraph.addArc(
					taskNodesToTaskGraphNodes[subtaskNodes.source(arc)],
					taskNodesToTaskGraphNodes[subtaskNodes.target(arc)]
				);
			}

			// Find sources and sinks
			std::vector<lemon::ListDigraph::Node> sources; // source are in m_taskGraph
			std::vector<lemon::ListDigraph::Node> sinks; // sink as well
			for (lemon::ListDigraph::NodeIt taskNode(subtaskNodes); taskNode != lemon::INVALID; ++taskNode) {
				// no in arc -> source
				int inArcCount = lemon::countInArcs(subtaskNodes, taskNode);
				if (inArcCount == 0) {
					sources.push_back(taskNodesToTaskGraphNodes[taskNode]);
				}
				// no out arc -> sink
				int outArcCount = lemon::countOutArcs(subtaskNodes, taskNode);
				if (outArcCount == 0) {
					sinks.push_back(taskNodesToTaskGraphNodes[taskNode]);
				}
			}
			SourceSinkMapping sourceSinkMapForCurrentNode;
			// If no sink or no source, it's certainly not a fucking DAG
			if (sources.size() == 0 || sinks.size() == 0) {
				std::stringstream ss;
				for (lemon::ListDigraph::ArcIt arc(subtaskNodes); arc != lemon::INVALID; ++arc) {
					ss << subtaskNodes.id(subtaskNodes.source(arc)) << " -> " << subtaskNodes.id(subtaskNodes.target(arc)) << std::endl;
				}
				throw InvalidArgumentException("Task graph of node has neither sources nor sinks. The graph must not contain circles.", ss.str()); // TODO: which node, which task, which what?
			}
			// If there are multiple sources, reduce them to one
			if (sources.size() > 1) {
				sourceSinkMapForCurrentNode.source = m_taskGraph.addNode();
				m_taskParentMap[sourceSinkMapForCurrentNode.source] = lemon::INVALID;
				m_taskFunctionMap[sourceSinkMapForCurrentNode.source] = nullptr;
				for (auto& source : sources) {
					m_taskGraph.addArc(sourceSinkMapForCurrentNode.source, source);
				}
			}
			else {
				sourceSinkMapForCurrentNode.source = sources[0];
			}
			// If there are multiple sinks, reduce them to one
			if (sinks.size() > 1) {
				sourceSinkMapForCurrentNode.sink = m_taskGraph.addNode();
				m_taskParentMap[sourceSinkMapForCurrentNode.sink] = lemon::INVALID;
				m_taskFunctionMap[sourceSinkMapForCurrentNode.sink] = nullptr;
				for (auto& sink : sinks) {
					m_taskGraph.addArc(sink, sourceSinkMapForCurrentNode.sink);
				}
			}
			else {
				sourceSinkMapForCurrentNode.sink = sinks[0];
			}
			// Set source-sink mapping
			sourceSinkMapping[depNode] = sourceSinkMapForCurrentNode;
		}
		else {
			std::unique_ptr<SimpleNodeTask> wrapper = std::make_unique<SimpleNodeTask>(pipelineNode);
			SourceSinkMapping sourceSinkMapForCurrentNode;
			lemon::ListDigraph::Node wrapperNode = m_taskGraph.addNode();
			sourceSinkMapForCurrentNode.sink = sourceSinkMapForCurrentNode.source = wrapperNode;

			m_taskFunctionMap[wrapperNode] = wrapper.get(); // assign subtask
			m_taskParentMap[wrapperNode] = depNode; // assign parent of subtask

			m_taskWrappers.push_back(std::move(wrapper));

			// Set source-sink mapping
			sourceSinkMapping[depNode] = sourceSinkMapForCurrentNode;
		}
	}

	// Connect sources and sinks according to dependencyGraph
	for (lemon::ListDigraph::ArcIt depArc(m_dependencyGraph); depArc != lemon::INVALID; ++depArc) {
		auto source = sourceSinkMapping[m_dependencyGraph.source(depArc)].sink;
		auto target = sourceSinkMapping[m_dependencyGraph.target(depArc)].source;
		m_taskGraph.addArc(source, target);
	}
}



void Pipeline::CalculateDependencyGraph() {
	// Erase all arcs from the graph
	lemon::ListDigraph::ArcIt arcIt(m_dependencyGraph);
	while (arcIt != lemon::INVALID) {
		auto deleteMe = arcIt;
		++arcIt;
		m_dependencyGraph.erase(deleteMe);
	}

	// Add all arcs to the graph accoring to inputPort->outputPort linkage
	// sidenote: the algorithm will not create duplicate arcs in the graph
	for (lemon::ListDigraph::NodeIt outerIt(m_dependencyGraph); outerIt != lemon::INVALID; ++outerIt) {
		for (auto innerIt = ++lemon::ListDigraph::NodeIt(outerIt); innerIt != lemon::INVALID; ++innerIt) {
			NodeBase* srcNode = m_nodeMap[outerIt].get();
			NodeBase* dstNode = m_nodeMap[innerIt].get();

			// Check if there's ANY link going in ANY direction b/w above nodes
			// FIRST direction
			if (IsLinked(srcNode, dstNode)) {
				// a link from srcNode to dstNode was found, add an arc
				m_dependencyGraph.addArc(outerIt, innerIt);
			}

			// REVERSE direction; FLIP nodes
			if (IsLinked(dstNode, srcNode)) {
				// a link from dstNode to srcNode was found, add an arc
				m_dependencyGraph.addArc(innerIt, outerIt);
			}
		}
	}
}


bool Pipeline::IsLinked(NodeBase* srcNode, NodeBase* dstNode) {
	for (size_t dstIn = 0; dstIn < dstNode->GetNumInputs(); dstIn++) {
		OutputPortBase* linked = dstNode->GetInput(dstIn)->GetLink();
		for (size_t srcOut = 0; srcOut < srcNode->GetNumOutputs(); srcOut++) {
			if (linked == srcNode->GetOutput(srcOut)) {
				// a link from srcNode to dstNode was found!
				return true;
			}
		}
	}
	// no link found
	return false;
}


const lemon::ListDigraph& Pipeline::GetDependencyGraph() const {
	return m_dependencyGraph;
}
const lemon::ListDigraph::NodeMap<std::shared_ptr<NodeBase>>& Pipeline::GetNodeMap() const {
	return m_nodeMap;
}

const lemon::ListDigraph& Pipeline::GetTaskGraph() const {
	return m_taskGraph;
}
const lemon::ListDigraph::NodeMap<GraphicsTask*>& Pipeline::GetTaskFunctionMap() const {
	return m_taskFunctionMap;
}

const lemon::ListDigraph::NodeMap<lemon::ListDigraph::NodeIt>& Pipeline::GetTaskParentMap() const {
	return m_taskParentMap;
}




//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------

void AssertThrow(bool condition, const std::string& text) {
	if (!condition) {
		throw InvalidArgumentException(text);
	}
};


NodeCreationInfo ParseNode(const rapidjson::GenericValue<rapidjson::UTF8<>>& obj) {
	using namespace rapidjson;

	NodeCreationInfo info;
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
				info.inputs.push_back({});
			}
			else if (inputs[i].IsString()) {
				info.inputs.push_back(inputs[i].GetString());
			}
			else if (inputs[i].IsInt64()) {
				info.inputs.push_back(std::to_string(inputs[i].GetInt64()));
			}
			else if (inputs[i].IsDouble()) {
				info.inputs.push_back(std::to_string(inputs[i].GetDouble()));
			}
			else {
				assert(false);
			}
		}
	}
	return info;
};


LinkCreationInfo ParseLink(const rapidjson::GenericValue<rapidjson::UTF8<>>& obj) {
	LinkCreationInfo info;

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
};


static StringErrorPosition GetStringErrorPosition(const std::string& str, size_t errorCharacter) {
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


static std::string SerializeNodesAndLinks(std::vector<NodeCreationInfo> nodes, std::vector<LinkCreationInfo> links) {
	std::stable_sort(nodes.begin(), nodes.end(), [](const NodeCreationInfo& lhs, const NodeCreationInfo& rhs) {
		return lhs.cl < rhs.cl;
	});
	std::stable_sort(nodes.begin(), nodes.end(), [](const NodeCreationInfo& lhs, const NodeCreationInfo& rhs) {
		return lhs.name < rhs.name;
	});

	std::stable_sort(links.begin(), links.end(), [](const LinkCreationInfo& lhs, const LinkCreationInfo& rhs) {
		return lhs.srcname < rhs.srcname;
	});
	std::stable_sort(links.begin(), links.end(), [](const LinkCreationInfo& lhs, const LinkCreationInfo& rhs) {
		return lhs.dstname < rhs.dstname;
	});

	using namespace rapidjson;
	Document doc;
	auto& alloc = doc.GetAllocator();
	doc.SetObject();

	doc.AddMember("nodes", kArrayType, alloc);
	doc.AddMember("links", kArrayType, alloc);
	auto& docNodes = doc["nodes"];
	auto& docLinks = doc["links"];

	// Add nodes to doc
	for (auto& node : nodes) {
		Value v;
		v.SetObject();
		v.AddMember("class", Value().SetString(node.cl.c_str(), alloc), alloc);
		if (node.id) {
			v.AddMember("id", node.id.value(), alloc);
		}
		if (node.name) {
			v.AddMember("name", Value().SetString(node.name.value().c_str(), alloc), alloc);
		}
		if (!node.inputs.empty()) {
			v.AddMember("inputs", kArrayType, alloc);
			for (const auto& input : node.inputs) {
				if (input) {
					v["inputs"].PushBack(Value().SetString(input.value().c_str(), alloc), alloc);
				}
				else {
					v["inputs"].PushBack(Value().SetObject(), alloc);
				}
			}
		}
		docNodes.PushBack(v, alloc);
	}

	// Add links to doc
	auto AddLinkMember = [&alloc](Value& v, const char* member, std::optional<int> num, std::optional<std::string> str) {
		if (num) {
			v.AddMember(GenericStringRef<char>(member), num.value(), alloc);
		}
		else {
			assert(str);
			v.AddMember(GenericStringRef<char>(member), Value().SetString(str.value().c_str(), alloc), alloc);
		}
	};

	for (auto& link : links) {
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


template <class T>
std::string GetInputPortValueHelper(const InputPortBase* port) {
	return std::to_string(dynamic_cast<const InputPort<T>*>(port)->Get());
}



} // namespace gxeng
} // namespace inl