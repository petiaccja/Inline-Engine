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
	Task();
	Task(const Task& rhs);
	Task(ElementaryTask subtask);
	Task(const std::vector<ElementaryTask>& parallelSubtasks);
	Task& operator=(const Task& rhs);

	Task& operator=(ElementaryTask subtask);
	void InitParallel(const std::vector<ElementaryTask>& subtasks);
	void InitSequential(const std::vector<ElementaryTask>& subtasks);
	
	void ResetNodes();
	void ResetSubtasks();

	lemon::ListDigraph m_nodes;
	lemon::ListDigraph::NodeMap<ElementaryTask> m_subtasks;
};


} // namespace gxeng
} // namespace inl
