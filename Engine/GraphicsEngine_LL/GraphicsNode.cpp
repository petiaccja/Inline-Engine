#include "GraphicsNode.hpp"


namespace inl::gxeng {


GraphicsNode::GraphicsNode()
	: m_taskMap(m_taskNodes)
{}



const lemon::ListDigraph& GraphicsNode::GetTaskGraph() const {
	return m_taskNodes;
}
const lemon::ListDigraph::NodeMap<GraphicsTask*>& GraphicsNode::GetTaskGraphMapping() const {
	return m_taskMap;
}


void GraphicsNode::SetTaskSingle(GraphicsTask* task) {
	m_taskNodes.clear();
	lemon::ListDigraph::Node node = m_taskNodes.addNode();
	m_taskMap[node] = task;
}


void GraphicsNode::SetTaskGraph(const lemon::ListDigraph& nodes, const lemon::ListDigraph::NodeMap<GraphicsTask*>& map) {
	m_taskNodes.clear();

	lemon::DigraphCopy<std::decay_t<decltype(nodes)>, std::decay_t<decltype(m_taskNodes)>> copy(nodes, m_taskNodes);
	copy.nodeMap(map, m_taskMap);
	copy.run();
}



} // namespace inl::gxeng