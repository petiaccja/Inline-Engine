#pragma once

#include <string>
#include <vector>
#include <iterator>
#include "../BaseLibrary/Graph/Node.hpp"

#include "GraphicsNode.hpp"

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
		NodeIterator(const Pipeline* parent, lemon::ListDigraph::NodeIt graphIt);
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
		const Pipeline* m_parent;
	};

	class SimpleNodeTask : public GraphicsTask {
	public:
		SimpleNodeTask(exc::NodeBase* subject) : m_subject(subject) {}
		void Setup(SetupContext& context) override { m_subject->Update(); }
		void Execute(RenderContext& context) override {}
	private:
		exc::NodeBase* m_subject;
	};

public:
	Pipeline();
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&);
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&);
	~Pipeline();

	void CreateFromDescription(const std::string& jsonDescription, GraphicsNodeFactory& factory);
	void CreateFromNodesList(const std::vector<std::shared_ptr<exc::NodeBase>> nodes);
	void Clear();

	NodeIterator Begin() const;
	NodeIterator End() const;

	const lemon::ListDigraph& GetDependencyGraph() const;
	const lemon::ListDigraph::NodeMap<std::shared_ptr<exc::NodeBase>>& GetNodeMap() const;
	const lemon::ListDigraph& GetTaskGraph() const;
	const lemon::ListDigraph::NodeMap<GraphicsTask*>& GetTaskFunctionMap() const;
	const lemon::ListDigraph::NodeMap<lemon::ListDigraph::NodeIt>& GetTaskParentMap() const;

	template <class T>
	void AddNodeMetaData() = delete;
	template <class T>
	void AddArcMetaData() = delete;
private:
	void CalculateTaskGraph();
	void CalculateDependencyGraph();
	bool IsLinked(exc::NodeBase* srcNode, exc::NodeBase* dstNode);

	lemon::ListDigraph m_dependencyGraph;
	lemon::ListDigraph::NodeMap<std::shared_ptr<exc::NodeBase>> m_nodeMap;
	lemon::ListDigraph m_taskGraph;
	lemon::ListDigraph::NodeMap<GraphicsTask*> m_taskFunctionMap;
	lemon::ListDigraph::NodeMap<lemon::ListDigraph::NodeIt> m_taskParentMap;

	std::vector<std::unique_ptr<SimpleNodeTask>> m_taskWrappers;
};



} // namespace gxeng
} // namespace inl
