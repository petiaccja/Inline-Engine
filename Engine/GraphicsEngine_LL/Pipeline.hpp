#pragma once

#include "Task.hpp"
#include <string>
#include <vector>
#include <iterator>
#include "../BaseLibrary/Graph/Node.hpp"

#ifdef _MSC_VER // disable lemon warnings
#pragma warning(push)
#pragma warning(disable: 4267)
#endif

#include <lemon/euler.h>
#include <lemon/list_graph.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace inl {
namespace gxeng {


class GraphicsNodeFactory;


class Pipeline {
public:
	class NodeIterator : public std::bidirectional_iterator_tag {
	private:
		friend class inl::gxeng::Pipeline;
		NodeIterator(Pipeline* parent, lemon::ListDigraph::NodeIt graphIt);
	public:
		NodeIterator();
		NodeIterator(const NodeIterator&) = default;
		NodeIterator& operator=(const NodeIterator&) = default;

		const exc::NodeBase& operator*();
		const exc::NodeBase* operator->();

		bool operator==(const NodeIterator&);
		bool operator!=(const NodeIterator&);

		NodeIterator& operator++();
		NodeIterator operator++(int);
	private:
		lemon::ListDigraph::NodeIt m_graphIt;
		Pipeline* m_parent;
	};


	using DynamicDeleter = std::default_delete<exc::NodeBase>; // calls delete
	using NoDeleter = struct { void operator()(exc::NodeBase*) { return; } }; // does nothing
public:
	Pipeline();
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&);
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&);
	~Pipeline();

	void CreateFromDescription(const std::string& jsonDescription, GraphicsNodeFactory& factory);
	template <class Deleter> 
	void CreateFromNodesList(const std::vector<exc::NodeBase*> nodes, Deleter nodeDeleter);
	void Clear();

	NodeIterator Begin();
	NodeIterator End();

	const lemon::ListDigraph& GetDependencyGraph() const;
	const lemon::ListDigraph::NodeMap<exc::NodeBase*>& GetNodeMap() const;
	const lemon::ListDigraph& GetTaskGraph() const;
	const lemon::ListDigraph::NodeMap<ElementaryTask>& GetTaskMap() const;

	template <class T>
	void AddNodeMetaData();
	template <class T>
	void AddArcMetaData();
private:
	void CalculateTaskGraph();
	void CalculateDependencyGraph();
	bool IsLinked(exc::NodeBase* srcNode, exc::NodeBase* dstNode);

	lemon::ListDigraph m_dependencyGraph;
	lemon::ListDigraph::NodeMap<exc::NodeBase*> m_nodeMap;
	lemon::ListDigraph m_taskGraph;
	lemon::ListDigraph::NodeMap<ElementaryTask> m_taskMap;

	std::function<void(exc::NodeBase*)> m_nodeDeleter;
};


template <class Deleter>
void Pipeline::CreateFromNodesList(const std::vector<exc::NodeBase*> nodes, Deleter nodeDeleter) {
	// assign pipeline nodes to graph nodes
	try {
		for (auto pipelineNode : nodes) {
			lemon::ListDigraph::Node graphNode = m_dependencyGraph.addNode();
			m_nodeMap[graphNode] = pipelineNode;
		}
	}
	catch (...) {
		for (auto pipelineNode : nodes) {
			nodeDeleter(pipelineNode);
		}
		throw;
	}

	// save deleter
	m_nodeDeleter = nodeDeleter;

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
		throw std::invalid_argument("Supplied nodes do not make a directed acyclic graph.");
	}
}



} // namespace gxeng
} // namespace inl
