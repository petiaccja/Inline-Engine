#include "Task.hpp"


namespace inl {
namespace gxeng {

Task::Task() : m_subtasks(m_nodes) {
	
}

Task::Task(const Task& rhs) : m_subtasks(m_nodes) {
	lemon::DigraphCopy<decltype(rhs.m_nodes), decltype(m_nodes)> copy(rhs.m_nodes, m_nodes);
	copy.nodeMap(rhs.m_subtasks, m_subtasks);
	copy.run();
}

Task& Task::operator=(const Task& rhs) {
	m_nodes.clear();
	lemon::DigraphCopy<decltype(rhs.m_nodes), decltype(m_nodes)> copy(rhs.m_nodes, m_nodes);
	copy.nodeMap(rhs.m_subtasks, m_subtasks);
	copy.run();
	return *this;
}


Task::Task(ElementaryTask subtask) : Task() {
	auto node = m_nodes.addNode();
	m_subtasks[node] = std::move(subtask);
}


Task::Task(const std::vector<ElementaryTask>& subtasks) : Task() {
	InitParallel(subtasks);
}


Task& Task::operator=(ElementaryTask subtask) {
	m_nodes.clear();
	auto node = m_nodes.addNode();
	m_subtasks[node] = std::move(subtask);
	return *this;
}


void Task::InitParallel(const std::vector<ElementaryTask>& subtasks) {
	ResetNodes();

	for (auto& subtask : subtasks) {
		auto node = m_nodes.addNode();
		m_subtasks[node] = subtask;
	}
}


void Task::InitSequential(const std::vector<ElementaryTask>& subtasks) {
	ResetNodes();

	lemon::ListDigraph::Node prevNode;

	for (auto& subtask : subtasks) {
		// create a node
		auto node = m_nodes.addNode();
		m_subtasks[node] = subtask;

		// add arc b/w prev and current node
		if (m_nodes.valid(prevNode)) {
			m_nodes.addArc(prevNode, node);
		}
	}
}


void Task::ResetNodes() {
	m_nodes.clear();
}


void Task::ResetSubtasks() {
	for (decltype(m_nodes)::NodeIt node(m_nodes); node != lemon::INVALID; ++node) {
		m_subtasks[node] = ElementaryTask();
	}
}




} // namespace gxeng
} // namespace inl