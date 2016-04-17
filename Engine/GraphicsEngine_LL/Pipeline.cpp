#include "Pipeline.hpp"
#include "GraphicsNodeFactory.hpp"
#include "GraphicsNode.hpp"

#include <rapidjson/rapidjson.h>
#include <cassert>
#include <map>

#include <lemon/dfs.h>



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
	return (m_parent->m_dependencyGraphNodes[m_graphIt]);
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
	: m_dependencyGraphNodes(m_dependencyGraph), m_taskGraphMaps(m_taskGraph)
{}

void Pipeline::CreateFromDescription(const std::string& jsonDescription) {
	throw std::logic_error("not implemented yet");
}

void Pipeline::SetFactory(GraphicsNodeFactory* factory) {
	m_factory = factory;
}

GraphicsNodeFactory* Pipeline::GetFactory() const {
	return m_factory;
}


Pipeline::NodeIterator Pipeline::Begin() {
	return{ this, lemon::ListDigraph::NodeIt(m_dependencyGraph) };
}

Pipeline::NodeIterator Pipeline::End() {
	return{ this, lemon::INVALID };
}


Pipeline::NodeIterator Pipeline::AddNode(const std::string& fullName) {
	exc::NodeBase* node = m_factory->CreateNode(fullName);
	if (node != nullptr) {
		lemon::ListDigraph::Node graphNode = m_dependencyGraph.addNode();
		m_dependencyGraphNodes[graphNode] = node;
		return NodeIterator(this, lemon::ListDigraph::NodeIt(m_dependencyGraph, graphNode));
	}
	else {
		throw std::runtime_error("Failed to create node.");
	}
}

void Pipeline::Erase(NodeIterator node) {
	if (node != End()) {
		assert(node.m_parent == this);
		assert(node.m_graphIt != lemon::INVALID);

		auto* nodePtr = const_cast<exc::NodeBase*>(node.operator->());
		delete nodePtr;
		m_dependencyGraph.erase(node.m_graphIt);
	}
}

void Pipeline::AddLink(NodeIterator srcNode, int srcPort, NodeIterator dstNode, int dstPort) {
	// Get pointers
	auto* srcPtr = const_cast<exc::NodeBase*>(srcNode.operator->());
	auto* dstPtr = const_cast<exc::NodeBase*>(dstNode.operator->());

	exc::OutputPortBase* srcPortPtr = srcPtr->GetOutput(srcPort);
	exc::InputPortBase* dstPortPtr = dstPtr->GetInput(dstPort);

	// Link ports
	bool success = srcPortPtr->Link(dstPortPtr);

	if (!success) {
		throw std::logic_error("Port types mismatching.");
	}

	// Add arc in dependency graph
	lemon::ListDigraph::Arc existingArc = lemon::findArc(m_dependencyGraph, srcNode.m_graphIt, dstNode.m_graphIt);
	if (existingArc == lemon::INVALID) {
		m_dependencyGraph.addArc(srcNode.m_graphIt, dstNode.m_graphIt);
	}
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

	lemon::ListDigraph::NodeMap<SourceSinkMapping> sourceSinkMapping(m_dependencyGraph); // map targets are in m_taskGraph

	// Iterate over each node in dependency graph, which graph is NOT expanded
	for (lemon::ListDigraph::NodeIt depNode(m_dependencyGraph); depNode != lemon::INVALID; ++depNode) {
		// Get pipeline node of this graph node
		exc::NodeBase* pipelineNode = m_dependencyGraphNodes[depNode];
		gxeng::GraphicsNode* graphicsPipelineNode = dynamic_cast<gxeng::GraphicsNode*>(pipelineNode);

		// Generate a Task from the pipeline node
		Task task;
		if (graphicsPipelineNode != nullptr) {
			task = graphicsPipelineNode->GetTask();
		}
		else {
			assert(pipelineNode != nullptr); // each graph node must have a pipeline node assigned
			task = ElementaryTask([pipelineNode](ExecutionContext ctx) {pipelineNode->Update(); return ExecutionResult();});
		}


		// Merge-copy Task's graph into m_taskGraph, create mapping of nodes
		// Copy nodes
		lemon::ListDigraph::NodeMap<decltype(m_taskGraph)::Node> taskNodesToTaskGraphNodes(task.m_nodes); // maps Task's nodes to m_taskGraph's nodes
		for (lemon::ListDigraph::NodeIt taskNode(task.m_nodes); taskNode != lemon::INVALID; ++taskNode) {
			auto taskGraphNode = m_taskGraph.addNode(); // add new node
			taskNodesToTaskGraphNodes[taskNode] = taskGraphNode; // map new node to task.m_nodes' corresponding node
			m_taskGraphMaps[taskGraphNode] = task.m_subtasks[taskNode]; // assign subtask
		}
		// Copy arcs
		for (lemon::ListDigraph::ArcIt arc(task.m_nodes); arc != lemon::INVALID; ++arc) {
			m_taskGraph.addArc(
				taskNodesToTaskGraphNodes[task.m_nodes.source(arc)],
				taskNodesToTaskGraphNodes[task.m_nodes.target(arc)]
			);
		}

		// Find sources and sinks
		std::vector<lemon::ListDigraph::Node> sources; // source are in m_taskGraph
		std::vector<lemon::ListDigraph::Node> sinks; // sink as well
		for (lemon::ListDigraph::NodeIt taskNode(task.m_nodes); taskNode != lemon::INVALID; ++taskNode) {
			// no in arc -> source
			if (lemon::countInArcs(task.m_nodes, taskNode) == 0) {
				sources.push_back(taskNodesToTaskGraphNodes[taskNode]);
			}
			// no out arc -> sink
			if (lemon::countOutArcs(task.m_nodes, taskNode) == 0) {
				sources.push_back(taskNodesToTaskGraphNodes[taskNode]);
			}
		}
		SourceSinkMapping sourceSinkMapForCurrentNode;
		// If no sink or no source, it's certainly not a fucking DAG
		if (sources.size() == 0 || sinks.size() == 0) {
			throw std::logic_error("Not a f'kin' DAG."); // TODO: which node, which task, which what?
		}
		// If there are multiple sources, reduce them to one
		if (sources.size() > 1) {
			sourceSinkMapForCurrentNode.source = m_taskGraph.addNode();
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

	// Connect sources and sinks according to dependencyGraph
	for (lemon::ListDigraph::ArcIt depArc(m_dependencyGraph); depArc != lemon::INVALID; ++depArc) {
		auto source = sourceSinkMapping[m_dependencyGraph.source(depArc)].sink;
		auto target = sourceSinkMapping[m_dependencyGraph.target(depArc)].source;
		m_taskGraph.addArc(source, target);
	}
}


void Pipeline::CalculateDependencies() {
	// Erase all arcs from the graph
	lemon::ListDigraph::ArcIt arcIt(m_dependencyGraph);
	while (arcIt != lemon::INVALID) {
		auto deleteMe = arcIt;
		++arcIt;
		m_dependencyGraph.erase(deleteMe);
	}


	// Check if there's any link between two pipeline nodes
	auto IsLinked = [](exc::NodeBase* srcNode, exc::NodeBase* dstNode)	-> bool {
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
	};

	// Add all arcs to the graph accoring to inputPort->outputPort linkage
	// sidenote: the algorithm will not create duplicate arcs in the graph
	for (lemon::ListDigraph::NodeIt outerIt(m_dependencyGraph); outerIt != lemon::INVALID; ++outerIt) {
		for (auto innerIt = ++lemon::ListDigraph::NodeIt(outerIt); outerIt != lemon::INVALID; ++outerIt) {
			exc::NodeBase* srcNode = m_dependencyGraphNodes[outerIt];
			exc::NodeBase* dstNode = m_dependencyGraphNodes[innerIt];

			// Check if there's ANY link going in ANY direction b/w above nodes
			// FIRST direction
			if (IsLinked(srcNode, dstNode)) {
				// a link from srcNode to dstNode was found, add an arc
				m_dependencyGraph.addArc(outerIt, innerIt);
			}

			// REVERSE direction; FLIP nodes
			if (IsLinked(dstNode, srcNode)) {
				// a link from srcNode to dstNode was found, add an arc
				m_dependencyGraph.addArc(outerIt, innerIt);
			}
		}
	}
}


} // namespace gxeng
} // namespace inl