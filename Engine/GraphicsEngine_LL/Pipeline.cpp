#undef IN
#undef OUT

#include "Pipeline.hpp"
#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/GraphEditor/GraphParser.hpp>

#include <cassert>
#include <optional>
#include <typeinfo>


namespace inl::gxeng {


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
	GraphParser parser;
	std::vector<std::shared_ptr<NodeBase>> nodeObjects;

	// Parse json.
	parser.Parse(jsonDescription);

	// Create nodes with initial values.
	for (auto& nodeDesc : parser.GetNodes()) {
		std::shared_ptr<NodeBase> nodeObject(factory.CreateNode(nodeDesc.cl));
		if (nodeDesc.name) {
			nodeObject->SetDisplayName(nodeDesc.name.value());
		}

		for (int i = 0; i < nodeObject->GetNumInputs() && i < nodeDesc.defaultInputs.size(); ++i) {
			if (nodeDesc.defaultInputs[i]) {
				nodeObject->GetInput(i)->SetConvert(nodeDesc.defaultInputs[i].value());
			}
		}

		nodeObjects.push_back(std::move(nodeObject));
	}

	// Link nodes above.
	for (auto& info : parser.GetLinks()) {
		NodeBase *src = nullptr, *dst = nullptr;
		OutputPortBase* srcp = nullptr;
		InputPortBase* dstp = nullptr;

		// Find src and dst nodes.
		size_t srcNodeIdx = parser.FindNode(info.srcid, info.srcname);
		size_t dstNodeIdx = parser.FindNode(info.dstid, info.dstname);

		src = nodeObjects[srcNodeIdx].get();
		dst = nodeObjects[dstNodeIdx].get();

		// Find src and dst ports.
		srcp = static_cast<OutputPortBase*>(parser.FindOutputPort(src, info.srcpidx, info.srcpname));
		dstp = static_cast<InputPortBase*>(parser.FindInputPort(dst, info.dstpidx, info.dstpname));

		// Link said ports
		srcp->Link(dstp);
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
	// Assign pipeline nodes to graph nodes.
	for (auto pipelineNode : nodes) {
		lemon::ListDigraph::Node graphNode = m_dependencyGraph.addNode();
		m_nodeMap[graphNode] = pipelineNode;
	}

	try {
		// Calculate dependency and task graphs.
		CalculateDependencyGraph();
		CalculateTaskGraph();

		// Check if graphs are DAGs.
		// Note: if task graph is a DAG => dep. graph must be a DAG.
		bool isTaskGraphDAG = lemon::dag(m_taskGraph);
		if (!isTaskGraphDAG) {
			throw InvalidArgumentException("Supplied nodes do not make a directed acyclic graph.");
		}

		// Remove transitive edges from task graph: long chains can now share the same command list.
		TransitiveReduction(m_taskGraph);
	}
	catch (...) {
		Clear();
		throw;
	}
}


std::string Pipeline::SerializeToJSON(const NodeFactory& factory) const {
	std::vector<const ISerializableNode*> nodes;
	for (lemon::ListDigraph::NodeIt it(m_dependencyGraph); it != lemon::INVALID; ++it) {
		const NodeBase* node = m_nodeMap[it].get();
		nodes.push_back(node);
	}

	auto FindName = [&](const ISerializableNode& node) {
		auto[group, className] = factory.GetFullName(typeid(node));
		return group + "/" + className;
	};

	GraphHeader header;
	header.contentType = "pipeline";
	return GraphParser::Serialize(nodes.data(), nullptr, nodes.size(), FindName, header);
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

void Pipeline::TransitiveReduction(lemon::ListDigraph& graph) {
	const int nodeCount = lemon::countNodes(graph);
	std::vector<bool> reachabilityData(nodeCount*nodeCount, false);
	auto reachbility = [&reachabilityData, nodeCount](int source, int target) {
		return reachabilityData[nodeCount*source + target];
	};

	// Get topological sort.
	lemon::ListDigraph::NodeMap<int> sortedMap(graph);
	bool isSortable = lemon::checkedTopologicalSort(graph, sortedMap);
	assert(isSortable);

	std::vector<lemon::ListDigraph::NodeIt> sortedNodes;
	for (lemon::ListDigraph::NodeIt nodeIt(graph); nodeIt != lemon::INVALID; ++nodeIt) {
		sortedNodes.push_back(nodeIt);
	}

	std::sort(sortedNodes.begin(), sortedNodes.end(), [&](auto n1, auto n2)
	{
		return sortedMap[n1] < sortedMap[n2];
	});

	// Iterate backwards to find transitive closure of the graph.
	for (auto it = sortedNodes.rbegin(); it != sortedNodes.rend(); ++it) {
		int index = sortedMap[*it];

		// Add current node to reachability list.
		reachbility(index, index) = true;

		// Add this node's reachables to the reachability of all predecessors.
		for (lemon::ListDigraph::InArcIt inArcIt(graph, *it); inArcIt != lemon::INVALID; ++inArcIt) {
			lemon::ListDigraph::Node pred = graph.source(inArcIt);
			int predIndex = sortedMap[pred];
			for (int i=0; i<nodeCount; ++i) {
				reachbility(predIndex, i) = reachbility(predIndex, i) || reachbility(index, i);
			}
		}
	}

	// For all edges, remove the edge if target can be reached from source without that edge.
	for (lemon::ListDigraph::ArcIt arcIt(graph); arcIt != lemon::INVALID; ++arcIt) {
		lemon::ListDigraph::Node source = graph.source(arcIt);
		lemon::ListDigraph::Node target = graph.target(arcIt);
		int targetIdx = sortedMap[target];
		
		bool reachable = false;
		for (lemon::ListDigraph::OutArcIt outArcIt(graph, source); outArcIt != lemon::INVALID && !reachable; ++outArcIt) {
			lemon::ListDigraph::Node jumper = graph.target(outArcIt);
			if (jumper != target) {
				int jumperIdx = sortedMap[jumper];
				for (int i = 0; i<nodeCount && !reachable; ++i) {
					reachable = reachable || reachbility(jumperIdx, targetIdx);
				}
			}
		}

		if (reachable) {
			graph.erase(arcIt);
		}
	}
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



} // namespace inl::gxeng