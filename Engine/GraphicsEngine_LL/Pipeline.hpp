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

	const lemon::ListDigraph& GetDependencyGraph() const;
	const lemon::ListDigraph::NodeMap<exc::NodeBase*>& GetNodeMap() const;
	const lemon::ListDigraph& GetTaskGraph() const;
	const lemon::ListDigraph::NodeMap<ElementaryTask>& GetTaskMap() const;

	template <class T>
	void AddNodeMetaData();
	template <class T>
	void AddArcMetaData();

	void CalculateTaskGraph_Dbg() { CalculateTaskGraph(); }
private:
	void CalculateTaskGraph() const; // const because m_taskGraph is lazy-evaluated, but GetTG should be const
	void CalculateDependencies();
	bool IsLinked(exc::NodeBase* srcNode, exc::NodeBase* dstNode);

	bool m_isTaskGraphDirty;

	lemon::ListDigraph m_dependencyGraph;
	lemon::ListDigraph::NodeMap<exc::NodeBase*> m_nodeMap;
	mutable lemon::ListDigraph m_taskGraph;
	mutable lemon::ListDigraph::NodeMap<ElementaryTask> m_taskMap;
	
	GraphicsNodeFactory* m_factory;
};



} // namespace gxeng
} // namespace inl
