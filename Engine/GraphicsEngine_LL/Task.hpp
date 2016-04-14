#pragma once

#include "../GraphicsApi_LL/ICommandList.hpp"

#include <functional>
#include <lemon/list_graph.h>


namespace inl {
namespace gxeng {


struct ExecutionContext {
	
};

struct ExecutionResult {
	inl::gxapi::ICommandList* commandList;
};

using ElementaryTask = std::function<ExecutionResult(const ExecutionContext&)>;




class Task {
public:
	using ElementaryTask = std::function<void()>;
public:
	Task();
	Task(ElementaryTask subtask);
	Task(const std::vector<ElementaryTask>& parallelSubtasks);

	void InitParallel(const std::vector<ElementaryTask>& subtasks);
	void InitSequential(const std::vector<ElementaryTask>& subtasks);
	
	void ResetProgress();
	void ResetNodes();
	void ResetSubtasks();

	lemon::ListDigraph m_nodes;
	lemon::ListDigraph::NodeMap<ElementaryTask> m_subtasks;
	lemon::ListDigraph::NodeMap<bool> m_progress;
};


} // namespace gxeng
} // namespace inl
