#pragma once

#include "Task.hpp"
#include <string>
#include <vector>
#include <iterator>
#include <lemon/list_graph.h>
#include "../BaseLibrary/Graph/Node.hpp"


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

public:
	Pipeline();
	~Pipeline() = default;

	void CreateFromDescription(const std::string& jsonDescription);

	void SetFactory(GraphicsNodeFactory* factory);
	GraphicsNodeFactory* GetFactory() const;


	NodeIterator Begin();
	NodeIterator End();
	NodeIterator AddNode(const std::string& fullName);
	void Erase(NodeIterator node);
	void AddLink(NodeIterator srcNode, int srcPort, NodeIterator dstNode, int dstPort);
	void Unlink(NodeIterator node, int inputPort);

	const lemon::ListDigraph& GetDependencyGraph() const { return m_dependencyGraph; }
	const lemon::ListDigraph::NodeMap<exc::NodeBase*>& GetNodeMap() const { return m_nodeMap; }
	const lemon::ListDigraph& GetTaskGraph() const { return m_taskGraph; }
	const lemon::ListDigraph::NodeMap<ElementaryTask>& GetTaskMap() const { return m_taskMap; }

	void CalculateTaskGraph_Dbg() { CalculateTaskGraph(); }
private:
	void CalculateTaskGraph();
	void CalculateDependencies();

	bool m_isTaskGraphDirty;

	lemon::ListDigraph m_dependencyGraph;
	lemon::ListDigraph::NodeMap<exc::NodeBase*> m_nodeMap;
	lemon::ListDigraph m_taskGraph;
	lemon::ListDigraph::NodeMap<ElementaryTask> m_taskMap;
	
	GraphicsNodeFactory* m_factory;
};



} // namespace gxeng
} // namespace inl
