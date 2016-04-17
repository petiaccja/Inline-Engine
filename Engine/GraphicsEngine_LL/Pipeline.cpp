#include "Pipeline.hpp"
#include "GraphicsNodeFactory.hpp"

#include <rapidjson/rapidjson.h>
#include <cassert>
#include <map>



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

void Pipeline::CreateFromDescription(const std::string& jsonDescription) {
	throw std::logic_error("not implemented yet");
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
	m_taskGraph.clear();
	for (lemon::ListDigraph::NodeIt node(m_dependencyGraph); node != lemon::INVALID; ++node) {
		
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