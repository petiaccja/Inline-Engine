#include "Task.hpp"


namespace inl {
namespace gxeng {

Task::Task() : m_subtasks(m_nodes), m_progress(m_nodes) {
	
}


Task::Task(ElementaryTask subtask) : Task() {
	auto node = m_nodes.addNode();
	m_subtasks[node] = std::move(subtask);
	m_progress[node] = false;
}


Task::Task(const std::vector<ElementaryTask>& subtasks) : Task() {
	InitParallel(subtasks);
}


void Task::InitParallel(const std::vector<ElementaryTask>& subtasks) {
	ResetNodes();

	for (auto& subtask : subtasks) {
		auto node = m_nodes.addNode();
		m_subtasks[node] = subtask;
		m_progress[node] = false;
	}
}


void Task::InitSequential(const std::vector<ElementaryTask>& subtasks) {
	ResetNodes();

	lemon::ListDigraph::Node prevNode;

	for (auto& subtask : subtasks) {
		// create a node
		auto node = m_nodes.addNode();
		m_subtasks[node] = subtask;
		m_progress[node] = false;

		// add arc b/w prev and current node
		if (m_nodes.valid(prevNode)) {
			m_nodes.addArc(prevNode, node);
		}
	}
}


void Task::ResetProgress() {
	for (decltype(m_nodes)::NodeIt node(m_nodes); node != lemon::INVALID; ++node) {
		m_progress[node] = false;
	}
}


void Task::ResetNodes() {
	m_nodes.clear();
}


void Task::ResetSubtasks() {
	for (decltype(m_nodes)::NodeIt node(m_nodes); node != lemon::INVALID; ++node) {
		m_subtasks[node] = ElementaryTask();
		m_progress[node] = false;
	}
}




} // namespace gxeng
} // namespace inl