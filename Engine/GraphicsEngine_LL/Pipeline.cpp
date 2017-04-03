#undef IN
#undef OUT

#include "Pipeline.hpp"
#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"

#include <rapidjson/rapidjson.h>
#include <cassert>


namespace inl {
namespace gxeng {


//------------------------------------------------------------------------------
// Iterator
//------------------------------------------------------------------------------

Pipeline::NodeIterator::NodeIterator(Pipeline* parent, lemon::ListDigraph::NodeIt graphIt)
	: m_graphIt(graphIt), m_parent(parent)
{}

Pipeline::NodeIterator::NodeIterator()
	: m_graphIt(lemon::INVALID), m_parent(nullptr)
{}

const exc::NodeBase& Pipeline::NodeIterator::operator*() {
	return *(operator->());
}

const exc::NodeBase* Pipeline::NodeIterator::operator->() {
	assert(m_parent != nullptr);
	assert(m_parent->m_dependencyGraph.valid(m_graphIt));
	return (m_parent->m_nodeMap[m_graphIt]);
}


bool Pipeline::NodeIterator::operator==(const NodeIterator& rhs) {
	return (m_parent == rhs.m_parent && m_graphIt == rhs.m_graphIt) || (m_parent == nullptr && rhs.m_parent == nullptr);
}

bool Pipeline::NodeIterator::operator!=(const NodeIterator& rhs) {
	return !(*this == rhs);
}


Pipeline::NodeIterator& Pipeline::NodeIterator::operator++() {
	++m_graphIt;
	return *this;
}

Pipeline::NodeIterator Pipeline::NodeIterator::operator++(int) {
	return NodeIterator(m_parent, ++lemon::ListDigraph::NodeIt(m_graphIt));
}


//------------------------------------------------------------------------------
// Pipeline
//------------------------------------------------------------------------------


Pipeline::Pipeline()
	: m_nodeMap(m_dependencyGraph), m_taskFunctionMap(m_taskGraph), m_taskParentMap(m_taskGraph, lemon::INVALID)
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
	throw std::logic_error("not implemented yet");
}


void Pipeline::Clear() {
	for (lemon::ListDigraph::NodeIt graphNode(m_dependencyGraph); graphNode != lemon::INVALID; ++graphNode) {
		m_nodeDeleter(m_nodeMap[graphNode]);
		m_nodeMap[graphNode] = nullptr; // not necessary, but better make sure
	}
	m_dependencyGraph.clear();
	m_taskGraph.clear();
}


Pipeline::NodeIterator Pipeline::Begin() {
	return{ this, lemon::ListDigraph::NodeIt(m_dependencyGraph) };
}


Pipeline::NodeIterator Pipeline::End() {
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
		exc::NodeBase* pipelineNode = m_nodeMap[depNode];
		assert(pipelineNode != nullptr); // each graph node must have a pipeline node assigned

		// Merge subgraph of graphics pipeline node into expanded task graph
		if (gxeng::GraphicsNode* graphicsPipelineNode = dynamic_cast<gxeng::GraphicsNode*>(pipelineNode)) {
			const lemon::ListDigraph& subtaskNodes = graphicsPipelineNode->GetTaskGraph();
			const lemon::ListDigraph::NodeMap<GraphicsTask*>& subtaskMap = graphicsPipelineNode->GetTaskGraphMapping();

			// Merge-copy Task's graph into m_taskGraph, create mapping of nodes
			// Copy nodes
			int numSubtasks = lemon::countNodes(subtaskNodes);
			if (numSubtasks == 0) {
				throw std::logic_error("Task has zero subtasks.");
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
				throw std::logic_error("Task graph of node has neither sources nor sinks.\n" + ss.str()); // TODO: which node, which task, which what?
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
			exc::NodeBase* srcNode = m_nodeMap[outerIt];
			exc::NodeBase* dstNode = m_nodeMap[innerIt];

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


bool Pipeline::IsLinked(exc::NodeBase* srcNode, exc::NodeBase* dstNode) {
	for (size_t dstIn = 0; dstIn < dstNode->GetNumInputs(); dstIn++) {
		exc::OutputPortBase* linked = dstNode->GetInput(dstIn)->GetLink();
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
const lemon::ListDigraph::NodeMap<exc::NodeBase*>& Pipeline::GetNodeMap() const {
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



} // namespace gxeng
} // namespace inl