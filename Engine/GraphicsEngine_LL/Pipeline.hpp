#pragma once

#include <string>
#include <vector>
#include <iterator>
#include <BaseLibrary/Graph/Node.hpp>
#include <BaseLibrary/Graph/NodeFactory.hpp>

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

		NodeBase& operator*() const;
		NodeBase* operator->() const;

		bool operator==(const NodeIterator&) const;
		bool operator!=(const NodeIterator&) const;

		NodeIterator& operator++();
		NodeIterator operator++(int) const;
	private:
		lemon::ListDigraph::NodeIt m_graphIt;
		const Pipeline* m_parent;
	};
	class ConstNodeIterator : protected NodeIterator {
		friend class inl::gxeng::Pipeline;
		ConstNodeIterator(const Pipeline* parent, lemon::ListDigraph::NodeIt graphIt) : NodeIterator(parent, graphIt) {}
	public:
		ConstNodeIterator() = default;
		ConstNodeIterator(const NodeIterator& rhs) : NodeIterator(rhs) {}
		const NodeBase& operator*() { return NodeIterator::operator*(); }
		const NodeBase* operator->() { return NodeIterator::operator->(); }
		bool operator==(const ConstNodeIterator& rhs) const { return NodeIterator::operator==(rhs); }
		bool operator!=(const ConstNodeIterator& rhs) const { return NodeIterator::operator!=(rhs); }
		using NodeIterator::operator++;
	};

	class SimpleNodeTask : public GraphicsTask {
	public:
		SimpleNodeTask(NodeBase* subject) : m_subject(subject) {}
		void Setup(SetupContext& context) override { m_subject->Update(); }
		void Execute(RenderContext& context) override {}
	private:
		NodeBase* m_subject;
	};

public:
	Pipeline();
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&);
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&);
	~Pipeline();

	void CreateFromDescription(const std::string& jsonDescription, GraphicsNodeFactory& factory);
	void CreateFromNodesList(const std::vector<std::shared_ptr<NodeBase>> nodes);
	std::string SerializeToJSON(const NodeFactory& factory) const;
	void Clear();

	NodeIterator begin();
	NodeIterator end();
	ConstNodeIterator begin() const;
	ConstNodeIterator end() const;
	ConstNodeIterator cbegin() const { return begin(); }
	ConstNodeIterator cend() const { return end(); }

	const lemon::ListDigraph& GetDependencyGraph() const;
	const lemon::ListDigraph::NodeMap<std::shared_ptr<NodeBase>>& GetNodeMap() const;
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
	bool IsLinked(NodeBase* srcNode, NodeBase* dstNode);
	static void TransitiveReduction(lemon::ListDigraph& graph);


	lemon::ListDigraph m_dependencyGraph;
	lemon::ListDigraph::NodeMap<std::shared_ptr<NodeBase>> m_nodeMap;
	lemon::ListDigraph m_taskGraph;
	lemon::ListDigraph::NodeMap<GraphicsTask*> m_taskFunctionMap;
	lemon::ListDigraph::NodeMap<lemon::ListDigraph::NodeIt> m_taskParentMap;

	std::vector<std::unique_ptr<SimpleNodeTask>> m_taskWrappers;
};



} // namespace gxeng
} // namespace inl
